procedure TfrmMain.Receive(Str, sFromIP: string);

function GetPuffer: string;
var
  i: integer;
begin
  Result := '';
  if Puffer.Anz > 0 then for i := 1 to Puffer.Anz do begin
    Result := Result + '#' + IntToStr(ord(Puffer.b[i]));
  end;
end;

var
  i: integer;
  t: double;
  abbruch, aender_leitung, aender_gb, aender_mb, test, merk, bolo: boolean;
  ch: char;
  s, sMessage: string;
  Anlage: TAnlage;
  protokoll: byte;

procedure PufferKuerzen(nAnzahl: integer);
var
  i, k, nCount: integer;
begin
  if puffer.anz > 0 then begin
    nCount := 0;
    for i := 0 to nAnzahl - 1 do begin
      k := i + nAnzahl;
      if (i > sizeof(puffer.b)) or (k > sizeof(puffer.b)) then begin
        break;
      end;
      Inc(nCount);
      puffer.b[i] := puffer.b[k];
    end;
    Dec(puffer.anz, nCount);
  end;
end;

procedure Send2Client(nAnzahl: integer);
var
  i: integer;
  BraxoniaClient: TBraxoniaClient;
  sNachricht: string;
begin
  if save.com.Netzfreigabe and (save.com.lstClient.Count > 0) and (nAnzahl > 0) then begin
    sNachricht := '';
    for i := 0 to nAnzahl - 1 do begin
      sNachricht := sNachricht + puffer.b[i];
    end;
    i := 0;
    while i < save.com.lstClient.Count do begin
      BraxoniaClient := TBraxoniaClient(save.com.lstClient.Items[i]);
      if not BraxoniaClient.SendString(sNachricht) then begin
        AddStatusLine(Translate('Client entfernt') + ' <' + BraxoniaClient.DNSName + '>');
        BraxoniaClient.Free;
        save.com.lstClient.Delete(i);
      end else Inc(i);
    end;
  end;
end;

var
  BraxoniaClient: TBraxoniaClient;
  nIndex: integer;
  tmpPara: TPara;
  bTemperaturOK: boolean;
begin
  connect_counter := 20;
  if save.com.ok and (Str <> '') then begin
    tbConnect.Visible := true;
    try
      try
{$ifdef control}
      control_haupt.Panel1.Caption:= Translate('M1290');
      control_haupt.Panel1.update;
{$endif}
      will_senden:=false;
      s := '';
      if Str <> '' then for i := 1 to Length(Str) do begin
        if puffer.anz >= SizeOf(puffer.b) then begin
          puffer.anz := 0;
          merk := save.anzeige;
          save.anzeige := false;
          AddStatusLine(Translate('M1010'));
          save.anzeige := merk;
        end;
        ch := Str[i];
        puffer.b[puffer.anz] := ch;
        Inc(puffer.anz);
        s := s + ' ' + IntToStr(Ord(ch));
      end;
      if (frmEmpfang <> nil) and (mnuEmpfangspuffernutzen.Checked) then begin
        try
          if Empfangspuffer <> nil then begin
            Empfangspuffer.Save('E' + s);
          end;
          frmEmpfang.Memo1.Lines.Add('E' + s);
        except
          on EOutOfResources do begin
            frmEmpfang.Memo1.Lines.Clear;
            frmEmpfang.Memo1.Lines.Add('E' + s);
          end;
          else begin
            AppException1.WriteError('Fehler beim Schreiben des Empfangspuffers');
            raise;
          end;
        end;
      end;
      { Auswertung des Empfanges }
      abbruch := false;

      repeat
        if puffer.anz > 0 then case Ord(puffer.b[0]) of
        6: begin // "Hallo" vom PIC empfangen
          if puffer.anz < 1 then begin
            puffer.anz := 0;
          end else begin
            { aktuellen Anlagentyp testen }
            Send2Client(1);
            repeat
              PufferKuerzen(1);
            until (puffer.anz = 0) or (Ord(puffer.b[0]) <> 6);
            SendeStatus;
          end;
        end;
        7, 5: begin // Message empfangen (von UDP-Client oder Server) und in der Statusline anzeigen
          sMessage := '';
          i := 1;
          while (i < puffer.anz) and (puffer.b[i] <> #0) do begin
            sMessage := sMessage + puffer.b[i];
            Inc(i);
          end;
          if puffer.b[0] = #7 then begin
            AddStatusLine(Translate('Message') + ' "' + sMessage + '" ' + Translate('vom Client') + ' ' + sFromIP + ' ' + Translate('empfangen'));
          end else if puffer.b[0] = #5 then begin
            nIndex := -1;
            for i := 0 to save.com.lstClient.Count - 1 do begin
              BraxoniaClient := TBraxoniaClient(save.com.lstClient.Items[i]);
              if (BraxoniaClient.IP = sFromIP) or (BraxoniaClient.DNSName = sFromIP) then  begin
                nIndex := i;
                break;
              end;
            end;
            if nIndex <> -1 then begin
              BraxoniaClient := TBraxoniaClient(save.com.lstClient.Items[nIndex]);
              BraxoniaClient.Update;
            end;
          end;
          PufferKuerzen(Length(sMessage) + 2);
        end;
        9: begin // Solldaten vom UDP-Server erhalten
          if puffer.anz < 54 then begin
            abbruch := true;
          end else begin
            // CRC-Berechnung
            CRC8 := TCRC8Berechnung.Create;
            try
              try
                for i := 0 to 52 do CRC8.Berechne(Ord(puffer.b[i]));
              except
                on E: Exception do begin
                  AppException1.WriteError('CRC-Fehler, ' + E.Message);
                end;
              end;
              if CRC8.Ergebnis <> Ord(puffer.b[53]) then begin
                puffer.anz := 0;
                AddStatusLine(Translate('M1060'));
                AppException1.WriteError('CRC-Fehler beim Empfang');
                exit;
              end;
            finally
              CRC8.Free;
            end;

            tmpPara := save.para.GetKopie;
            try
              // 1. Byte Maischbottich 1
              save.para.mb1.ruehrwerk := (Ord(puffer.b[1]) and (1 shl 0)) <> 0;
              save.para.mb1.heizung.an := (Ord(puffer.b[1]) and (1 shl 1)) <> 0;

              // 2.-5. Byte Solltemperatur Maischbottich 1
              BerechneT(puffer.b[2], puffer.b[3], puffer.b[4], puffer.b[5], save.para.mb1.temp.soll, bTemperaturOK);

              // 6. Byte Maischbottich 2
              save.para.mb2.ruehrwerk := (Ord(puffer.b[6]) and (1 shl 0)) <> 0;
              save.para.mb2.heizung.an := (Ord(puffer.b[6]) and (1 shl 1)) <> 0;

              // 7.-10. Byte Solltemperatur Maischbottich 2
              BerechneT(puffer.b[7], puffer.b[8], puffer.b[9], puffer.b[10], save.para.mb2.temp.soll, bTemperaturOK);

              // 11. Byte Schalter Gärbehälter
              for i := 1 to 8 do begin
                save.para.gb[i].an := (Ord(puffer.b[11]) and (1 shl (i - 1))) <> 0;
              end;

              // 12. Byte bis 43. Byte Solltemperatur Gärbehälter
              for i := 1 to 8 do begin
                BerechneT(puffer.b[12 + (i - 1) * 4], puffer.b[13 + (i - 1) * 4], puffer.b[14 + (i - 1) * 4], puffer.b[15 + (i - 1) * 4], save.para.gb[i].temp.soll, bTemperaturOK);
              end;

              // 44. Byte Rest Schalter
              save.para.warmwasser.an := (Ord(puffer.b[44]) and (1 shl 0)) <> 0;
              save.para.membran := (Ord(puffer.b[44]) and (1 shl 1)) <> 0;
              save.para.Abwasser := (Ord(puffer.b[44]) and (1 shl 2)) <> 0;
              save.para.kuehlwasserpumpe := (Ord(puffer.b[44]) and (1 shl 3)) <> 0;
              save.para.kuehlaggregat.an := (Ord(puffer.b[44]) and (1 shl 4)) <> 0;

              // 45..48. Byte Soll Warmwasser
              BerechneT(puffer.b[45], puffer.b[46], puffer.b[47], puffer.b[48], save.para.warmwasser.temp.soll, bTemperaturOK);

              // 49..52. Byte Soll Kühlaggregat
              BerechneT(puffer.b[49], puffer.b[50], puffer.b[51], puffer.b[52], save.para.kuehlaggregat.temp.soll, bTemperaturOK);

              if save.Para.ifMBStatus(tmpPara) then begin
                if save.anlage = l100 then begin
                  frmSteuermonitor100.ShowMB;
                end else begin
                  frmSteuermonitor50.ShowMB;
                end;
              end;

              if save.Para.ifGBStatus(tmpPara) then begin
                if save.anlage = l100 then begin
                  frmSteuermonitor100.ShowGB;
                end else begin
                  frmSteuermonitor50.ShowGB;
                end;
              end;

              if save.Para.ifSchalterStatus(tmpPara) then begin
                if save.anlage = l100 then begin
                  frmSteuermonitor100.SchalterStatus(nil);
                end else begin
                  frmSteuermonitor50.SchalterStatus(nil);
                end;
              end;
            finally
              tmpPara.Free;
            end;

            PufferKuerzen(53);
          end;
        end;
        80, 81: begin // Gärkeller
          puffer.anz:=0;
          save.com.ok:=false;
          frmZeigeNachricht := TfrmZeigeNachricht.Create(Application);
          frmZeigeNachricht.Nachricht := '1380';
          frmZeigeNachricht.Show;
        end;
        33, 49, 65: begin // Teil 2 des Temperaturblockes, Protokoll 2, 3.7.1998
          aender_mb:=false;
          aender_gb:=false;
          if save.com.protokoll <> 2 then begin
            save.com.ok := false;
            frmNachrichtendialog := TfrmNachrichtendialog.Create(Application);
            frmNachrichtendialog.Caption := Translate('Info');
            frmNachrichtendialog.Button3.Caption := Translate('OK');
            frmNachrichtendialog.Button3.OnClick := frmNachrichtendialog.Button3Click;
            frmNachrichtendialog.Button1.Visible := false;
            frmNachrichtendialog.Button2.Visible := false;
            frmNachrichtendialog.Label1.Caption := Translate('Protokollfehler, Protokoll 2 erwartet.');
            frmNachrichtendialog.Show;
          end;
          if puffer.anz < 19 then begin
            abbruch := true;
          end else begin
            // CRC-Berechnung
            CRC8 := TCRC8Berechnung.Create;
            try
              try
                for i := 0 to 17 do CRC8.Berechne(Ord(puffer.b[i]));
              except
                on E: Exception do begin
                  AppException1.WriteError('CRC-Fehler, ' + E.Message);
                end;
              end;
              if CRC8.Ergebnis <> Ord(puffer.b[18]) then begin
                puffer.anz := 0;
                AddStatusLine(Translate('M1060'));
                AppException1.WriteError('CRC-Fehler beim Empfang');
                exit;
              end;
            finally
              CRC8.Free;
            end;

            // Temperaturen einlesen
            try
              BerechneT(Ord(puffer.b[1]), Ord(puffer.b[2]), Ord(puffer.b[3]), Ord(puffer.b[4]), t, bTemperaturOK);
              if (t <> save.para.gb[7].temp.ist) and bTemperaturOK then begin
                DatenSchreiben(10);
                save.para.gb[7].temp.ist := t;
                aender_gb := true;
              end;
              BerechneT(Ord(puffer.b[5]), Ord(puffer.b[6]), Ord(puffer.b[7]), Ord(puffer.b[8]), t, bTemperaturOK);
              if (t <> save.para.gb[8].temp.ist) and bTemperaturOK then begin
                DatenSchreiben(11);
                save.para.gb[8].temp.ist := t;
                aender_gb:=true;
              end;
{             Position 15 z.Zt. unbelegt, 5.7.1998, Leipzig
              t:=berechne_t(puffer.b^[10],puffer.b^[11],puffer.b^[12],puffer.b^[13]);
              if t<>save.para.wuerze.ist then begin
                aender_mb:=true;
                save.para.wuerze.ist:=t;
              end;}
              BerechneT(Ord(puffer.b[13]), Ord(puffer.b[14]), Ord(puffer.b[15]), Ord(puffer.b[16]), t, bTemperaturOK);
              if (t <> save.para.wuerze.ist) and bTemperaturOK then begin
                aender_mb := true;
                save.para.wuerze.ist := t;
              end;
            except
              on E: Exception do begin
                AppException1.WriteError(E.Message + ', Größe des Empfangspuffers:' + IntToStr(Puffer.Anz)+ #13#10 + 'Empfangspuffer:' + GetPuffer);
              end;
            end;

            // Bildschirme aktualisieren
            if aender_gb then begin
              if save.anlage = l100 then begin
                frmSteuermonitor100.ShowGB
              end else begin
                frmSteuermonitor50.ShowGB;
              end;
            end;
            if aender_mb then begin
              if save.anlage = l100 then begin
                frmSteuermonitor100.ShowSchalter(nil)
              end else begin
                frmSteuermonitor50.ShowSchalter(nil);
              end;
              if save.Abarbeitung = 2 then begin
                try
                  if Rezept1.frmAbarbeitung <> nil then begin
                    TfrmAbarbeitung(Rezept1.frmAbarbeitung).ZeigeTemperaturen;
                  end;
                except
                  on E: Exception do begin
                    Rezept1.frmAbarbeitung := nil;
                    AppException1.WriteError('Receive Rezept1: ' + E.Message);
                  end;
                end;
                try
                  if Rezept2.frmAbarbeitung <> nil then begin
                    TfrmAbarbeitung(Rezept2.frmAbarbeitung).ZeigeTemperaturen;
                  end;
                except
                  on E: Exception do begin
                    Rezept2.frmAbarbeitung := nil;
                    AppException1.WriteError('Receive Rezept2: ' + E.Message);
                  end;
                end;
              end;
            end;
            Send2Client(19);
            PufferKuerzen(19);
          end;
        end;
        // 4=20l V.1,8=50l V.1,16=100l V.1, 32=20l V.2, 48=50l V.2, 64=100l V.2
        4, 8, 16, 32, 48, 64: begin
          if (Ord(puffer.b[0]) in [32, 48, 64]) then protokoll := 2 else protokoll := 1;
          if protokoll <> save.com.protokoll then begin
            UmschaltungProtokoll(protokoll);
          end;
          case Ord(puffer.b[0]) of
            4, 32: Anlage := l20;
            8, 48: Anlage := l50;
            16, 64: Anlage := l100;
            else Anlage := l20;
          end;
          if anlage <> save.anlage then begin
            UmschaltungAnlage(save.anlage, anlage);
            save.anlage := anlage;
          end;
          if puffer.anz < 44 then begin
            abbruch := true;
          end else begin
            CRC8 := TCRC8Berechnung.Create;
            try
              for i := 0 to 42 do CRC8.Berechne(Ord(puffer.b[i]));
              if CRC8.Ergebnis <> Ord(puffer.b[43]) then begin
                puffer.anz := 0;
                AddStatusLine(Translate('M1060'));
                exit;
              end;
            finally
              CRC8.Free;
            end;
            { Test, ob Membranpumpe auf Grund von Kondenswasser angeschaltet werden muß, 11.6.1997 }
            aender_mb := false;
            if (Ord(puffer.b[41]) and 128 = 0) and (not save.para.abwasser) then begin
              save.para.kondenswasser.count := 8;
              save.para.kondenswasser.an := true;
              save.para.abwasser := true;
              aender_mb := true;
            end;
            if save.para.kondenswasser.an then begin
              if save.para.kondenswasser.Count > 0 then dec(save.para.kondenswasser.count) else begin
                save.para.abwasser := false;
                save.para.kondenswasser.an := false;
              end;
            end;
            if save.com.protokoll in [1] then SendeStatus;
            { Abfrage Rührwassersensoren }
            test := save.para.mb1.ruehrwasser_sensor.an;
            save.para.mb1.ruehrwasser_sensor.an := true;// (puffer.b[42] and 1)<>0; ausgeschaltet am 19.6.1997
            if test<>save.para.mb1.ruehrwasser_sensor.an then aender_mb:=true;
            if save.para.mb1.ruehrwasser_sensor.an then begin
              { Max Datenpakete wird abgewartet, bis Alarm geschlagen wird, 30.4.97 }
              save.para.mb1.ruehrwasser_sensor.Count := save.para.mb1.ruehrwasser_sensor.max;
            end;
            test := save.para.mb2.ruehrwasser_sensor.an;
            save.para.mb2.ruehrwasser_sensor.an := true; // (puffer.b[42] and 1)<>0; ausgeschaltet am 19.6.1997
            if test<>save.para.mb2.ruehrwasser_sensor.an then aender_mb:=true;
            if save.anlage = l100 then begin
              if save.para.mb2.ruehrwasser_sensor.an then begin
                { Max Datenpakete wird abgewartet, bis Alarm geschlagen wird, 30.4.97 }
                save.para.mb2.ruehrwasser_sensor.Count := save.para.mb2.ruehrwasser_sensor.max;
              end;
            end;
            aender_gb := false;
            try
              for i := 1 to 6 do begin
                BerechneT(Ord(puffer.b[(i - 1) * 4 + 1]),
                          Ord(puffer.b[(i - 1) * 4 + 2]),
                          Ord(puffer.b[(i - 1) * 4 + 3]),
                          Ord(puffer.b[(i - 1) * 4 + 4]),
                          t, bTemperaturOK);
                {$ifdef control}
                TLabel(control_haupt.FindComponent('GB' + IntToStr(i))).Caption := Einheit.Temperatur(t);
                {$endif}
                if save.para.gb[i].temp.ist<>t then begin
                  DatenSchreiben(i+3);
                  save.para.gb[i].temp.ist:=t;
                  aender_gb:=true;
                end;
              end;
              BerechneT(Ord(puffer.b[25]), Ord(puffer.b[26]), Ord(puffer.b[27]), Ord(puffer.b[28]), t, bTemperaturOK);
              {$ifdef control}
              control_haupt.istmb.caption := einheit.temperatur(t);
              {$endif}
              if save.para.mb1.temp.ist <> t then begin
                DatenSchreiben(2);
                save.para.mb1.temp.ist := t;
                aender_mb := true;
              end;
              BerechneT(Ord(puffer.b[29]), Ord(puffer.b[30]), Ord(puffer.b[31]), Ord(puffer.b[32]), t, bTemperaturOK);
              {$ifdef control}
              control_haupt.istww.caption := einheit.temperatur(t);
              {$endif}
              if save.para.warmwasser.temp.ist <> t then begin
                DatenSchreiben(0);
                save.para.warmwasser.temp.ist := t;
                aender_mb := true;
              end;
              BerechneT(Ord(puffer.b[33]), Ord(puffer.b[34]), Ord(puffer.b[35]), Ord(puffer.b[36]), t, bTemperaturOK);
              {$ifdef control}
              control_haupt.istka.caption := einheit.temperatur(t);
              {$endif}
              if save.para.kuehlaggregat.temp.ist <> t then begin
                DatenSchreiben(1);
                save.para.kuehlaggregat.temp.ist := t;
                aender_mb := true;
              end;
              BerechneT(Ord(puffer.b[37]), Ord(puffer.b[38]), Ord(puffer.b[39]), Ord(puffer.b[40]), t, bTemperaturOK);
              // noch frei Messung
              if (save.anlage = l100) and (save.para.mb2.temp.ist<>t) then begin
                DatenSchreiben(3);
                save.para.mb2.temp.ist := t;
                aender_mb := true;
              end;
            except
              on E: Exception do begin
                AppException1.WriteError(E.Message + ', Größe des Empfangspuffers:' + IntToStr(Puffer.Anz) + #13#10 + 'Empfangspuffer:' + GetPuffer);
              end;
            end;
            aender_leitung := false;
            bolo := (Ord(puffer.b[41]) and 8) = 0;
            {$ifdef control}
            control_haupt.v9.checked := bolo;
            {$endif}
            if bolo <> save.para.v9 then begin
              aender_leitung := true;
              save.para.v9 := bolo;
            end;
            bolo := (Ord(puffer.b[41]) and 4) = 0;
            {$ifdef control}
            control_haupt.v8.checked := bolo;
            {$endif}
            if bolo <> save.para.v8 then begin
              aender_leitung := true;
              save.para.v8 := bolo;
            end;
            bolo := (Ord(puffer.b[41]) and 2) = 0;
            {$ifdef control}
            control_haupt.v7.checked := bolo;
            {$endif}
            if bolo <> save.para.v7 then begin
              aender_leitung := true;
              save.para.v7 := bolo;
            end;
            bolo := (Ord(puffer.b[41]) and 1) = 0;
            {$ifdef control}
            control_haupt.v6.checked := bolo;
            {$endif}
            if bolo <> save.para.v6 then begin
              aender_leitung := true;
              save.para.v6 := bolo;
            end;
            if aender_gb then begin
              if save.anlage = l100 then begin
                frmSteuermonitor100.ShowGB
              end else begin
                frmSteuermonitor50.ShowGB;
              end;
            end;
            if aender_mb then begin
              if save.anlage = l100 then begin
                frmSteuermonitor100.ShowSchalter(nil)
              end else begin
                frmSteuermonitor50.ShowSchalter(nil);
              end;
            end;
            if aender_leitung then begin
              if save.anlage = l100 then begin
                frmSteuermonitor100.ShowVentil(nil);
                frmSteuermonitor100.ShowLeitung(nil);
              end else begin
                frmSteuermonitor50.ShowVentil(nil);
                frmSteuermonitor50.ShowLeitung(nil);
              end;
            end;
            // für 20l und 50l sowie 100l mb1
            if (not save.para.mb1.ruehrwasser_sensor.an) and save.para.mb1.heizung.an then begin
              dec(save.para.mb1.ruehrwasser_sensor.count);
              if save.para.mb1.ruehrwasser_sensor.count <= 0 then begin
                save.para.mb1.ruehrwerk := false;
                save.para.mb1.heizung.an := false;
                save.para.mb1.kuehlung.an := false;
                if save.anlage = l100 then begin

                end else frmSteuermonitor50.ShowSchalter(nil);
                frmZeigeNachricht := TfrmZeigeNachricht.Create(Application);
                frmZeigeNachricht.Nachricht := '1230';
                frmZeigeNachricht.Show;
              end;
            end;
            // für die 100l-Anlage
            if (save.anlage = l100) and (not save.para.mb2.ruehrwasser_sensor.an) and save.para.mb2.heizung.an then begin
              dec(save.para.mb2.ruehrwasser_sensor.count);
              if save.para.mb2.ruehrwasser_sensor.count <= 0 then begin
                save.para.mb2.ruehrwerk := false;
                save.para.mb2.heizung.an := false;
                save.para.mb2.kuehlung.an := false;
                frmZeigeNachricht := TfrmZeigeNachricht.Create(Application);
                frmZeigeNachricht.Nachricht := '1230';
                frmZeigeNachricht.Show;
              end;
            end;
            Send2Client(44);
            PufferKuerzen(44);
          end;
        end;
        240: begin
          PufferKuerzen(1);
        end;
        else begin
          puffer.anz:=0;
        end;
      end;
      until (puffer.anz = 0) or abbruch;
      except
        on ezerodivide do begin
          puffer.anz := 0;
          raise;
        end;
        else begin
          AddStatusLine(Translate('Exception in Receive'));
        end;
      end;
    finally
      {$ifdef control}
      control_haupt.panel1.caption:='';
      control_haupt.panel1.update;
      {$endif}
    end;
  end;
end;
















{Hier der Code zum Senden:}

procedure TfrmMain.SendString(s: string);
var
  i: integer;
  ss, ss20: string;
  pc: pchar;
  buffer: array[0..1000] of char;
begin
  if aufsendung then exit;
  try
    aufsendung:=true;
    {$ifdef control}
    control_haupt.Panel1.Caption := Translate('M1280');
    control_haupt.Panel1.Update;
    {$endif}
    ss20:='';
    for i:=1 to length(s) do ss20:=ss20+' '+inttostr(ord(s[i]));
    if (frmEmpfang <> nil) and mnuEmpfangspufferanzeigen.Checked then begin
      try
        if Empfangspuffer <> nil then begin
          Empfangspuffer.Save('S' + ss20);
        end;
        frmEmpfang.Memo1.Lines.Add('S' + ss20);
      except
        on EOutOfResources do begin
          frmEmpfang.Memo1.Lines.Clear;
          frmEmpfang.Memo1.Lines.Add('S' + ss20);
        end;
        else begin
          AppException1.WriteError('Fehler beim Schreiben des Empfangspuffers');
        end;
      end;
    end;
    if length(s) > 0 then begin
      getmem(pc,length(s)+1);
      strpcopy(pc,s);
      ss:=s;
      if (save.com.port = 'UDP') and (save.com.offen) and (Length(ss) <= sizeof(buffer)) then begin
        for i := 1 to Length(ss) do begin
          buffer[i - 1] := ss[i];
        end;
        try
          NMUDPClient.SendBuffer(buffer, Length(ss));
        except
          save.com.offen := false;
        end;
      end;
      if ComPort1.Connected then begin
        ComPort1.WriteStr(ss);
      end else if save.Simulator <> nil then begin
        save.Simulator.WriteStr(ss);
      end;
      freemem(pc,length(s)+1);
    end;
  finally
    aufsendung:=false;
    {$ifdef control}
    control_haupt.panel1.caption:='';
    {$endif}
  end;
end;












{Code um aus den Daten der Temperatursensoren die Temperatur auszurechnen:}

procedure BerechneT(t_lsb, t_msb, t_rem, t_per: byte; var nTemperatur: double; var bOK: boolean);
var
  nMerk: double;
begin
  nMerk := nTemperatur;
  bOK := true;
  if t_per = 0 then begin
    bOK := false;
  end else begin
    if t_msb = 0 then begin
      nTemperatur := t_lsb / 2;
      if t_per <> 0 then nTemperatur := nTemperatur - 0.25 + (t_per - t_rem) / t_per;
    end else begin
      nTemperatur := - ((not t_lsb) + 1) / 2;
      if t_per <> 0 then nTemperatur := nTemperatur - 0.25 + (t_per - t_rem) / t_per;
    end;
    nTemperatur := round(nTemperatur * 10) / 10;
  end;
  if (nTemperatur < -10) or (nTemperatur > 110) then begin
    nTemperatur := nMerk;
    bOK := false;
  end;
end;





{CRC-Berechnung wird verwendet}

TCRC8Berechnung = class
  public
    BitX: array[0..7] of TBit;

    constructor Create;

    procedure Berechne(w: byte);
    function Ergebnis: byte;
end;
constructor TCRC8Berechnung.Create;
var
  i: byte;
begin
  inherited;
  
  for i := 0 to 7 do begin
    BitX[i] := 0;
  end;
end;

procedure TCRC8Berechnung.Berechne(w: byte);
var
  i: byte;
  bita, bit: TBit;
begin
  for i := 0 to 7 do begin
    if w and (1 shl i) = 0 then bit := 0 else bit := 1;
    bita := bitx[0] xor bit;
    BitX[0] := BitX[1];
    BitX[1] := BitX[2];
    BitX[2] := bita xor BitX[3];
    BitX[3] := bita xor BitX[4];
    BitX[4] := BitX[5];
    BitX[5] := BitX[6];
    BitX[6] := BitX[7];
    BitX[7] := bita;
  end;
end;

function TCRC8Berechnung.Ergebnis: byte;
var
  shift, k: byte;
begin
  shift := 0;
  for k := 0 to 7 do if BitX[k] = 1 then shift := shift or (1 shl k);
  ergebnis := shift;
end;