void Receive(string Str, string sFromIP) {
    int i;
    double t;
    bool abbruch, aender_leitung, aender_gb, aender_mb, test, merk, bolo;
    char ch;
    string s, sMessage;
    TAnlage Anlage;
    byte protokoll;

    connect_counter = 20;
    if (save.com.ok && !Str.empty()) {
        tbConnect.Visible = true;
        try {
            try {
#ifdef control
                control_haupt.Panel1.Caption = Translate("M1290");
                control_haupt.Panel1.update();
#endif
                will_senden = false;
                s = "";
                if (!Str.empty()) {
                    for (i = 0; i < Str.length(); i++) {
                        if (puffer.anz >= SizeOf(puffer.b)) {
                            puffer.anz = 0;
                            merk = save.anzeige;
                            save.anzeige = false;
                            AddStatusLine(Translate("M1010"));
                            save.anzeige = merk;
                        }
                        ch = Str[i];
                        puffer.b[puffer.anz] = ch;
                        puffer.anz++;
                        s += ' ' + to_string((int)ch);
                    }
                }
                if (frmEmpfang != nullptr && mnuEmpfangspuffernutzen.Checked) {
                    try {
                        if (Empfangspuffer != nullptr) {
                            Empfangspuffer.Save("E" + s);
                        }
                        frmEmpfang.Memo1.Lines.Add("E" + s);
                    }
                    catch (EOutOfResources& e) {
                        frmEmpfang.Memo1.Lines.Clear();
                        frmEmpfang.Memo1.Lines.Add("E" + s);
                    }
                    catch (...) {
                        AppException1.WriteError("Fehler beim Schreiben des Empfangspuffers");
                        throw;
                    }
                }
                // Auswertung des Empfanges
                abbruch = false;
                do {
                    if (puffer.anz > 0) {
                        switch ((int)puffer.b[0]) {
                            case 6: // "Hallo" vom PIC empfangen
                                if (puffer.anz < 1) {
                                    puffer.anz = 0;
                                }
                                else {
                                    // aktuellen Anlagentyp testen
                                    Send2Client(1);
                                    do {
                                        PufferKuerzen(1);
                                    } while ((puffer.anz == 0) || ((int)puffer.b[0] != 6));
                                    SendeStatus();
                                }
                                break;
                            case 7:
                            case 5: // Message empfangen (von UDP-Client oder Server) und in der Statusline anzeigen
                                sMessage = "";
                                i = 1;
                                while ((i < puffer.anz) && (puffer.b[i] != '\0')) {
                                    sMessage += puffer.b[i];
                                    i++;
                                }
                                if (puffer.b[0] == 7) {
                                    AddStatusLine(Translate("Message") + " \"" + sMessage + "\" " + Translate("vom Client") + " " + sFromIP + " " + Translate("empfangen"));
                                }
                                else if (puffer.b[0] == 5) {
                                    int nIndex = -1;
                                    for (i = 0; i < save.com.lstClient.Count; i++) {
                                        BraxoniaClient = TBraxoniaClient(save.com.lstClient.Items[i]);
                                        if ((BraxoniaClient.IP == sFromIP) || (BraxoniaClient.DNSName == sFromIP)) {
                                            nIndex = i;
                                            break;
                                        }
                                    }
                                    if (nIndex != -1) {
                                        BraxoniaClient = TBraxoniaClient(save.com.lstClient.Items[nIndex]);
                                        BraxoniaClient.Update();
                                    }
                                }
                                PufferKuerzen(sMessage.length() + 2);
                                break;
                            case 9: // Solldaten vom UDP-Server erhalten
                                if (puffer.anz < 54) {
                                    abbruch = true;
                                }
                                else {
                                    // CRC-Berechnung
                                    CRC8 = TCRC8Berechnung.Create();
                                    try {
                                        for (i = 0; i < 52; i++) {
                                            CRC8.Berechne((int)puffer.b[i]);
                                        }
                                        if (CRC8.Ergebnis != (int)puffer.b[53]) {
                                            puffer.anz = 0;
                                            AddStatusLine(Translate("M1060"));
                                            AppException1.WriteError("CRC-Fehler beim Empfang");
                                            return;
                                        }
                                    }
                                    catch (Exception& e) {
                                        AppException1.WriteError("CRC-Fehler, " + e.Message);
                                    }
                                    // neue Anlagedaten einlesen
                                    test = true;
                                    do {
                                        PufferKuerzen(1);
                                        if (puffer.anz > 53) {
                                            if (puffer.b[0] == 9) {
                                                test = false;
                                            }
                                            else {
                                                AddStatusLine(Translate("M1060"));
                                                test = false;
                                                abbruch = true;
                                            }
                                        }
                                        else {
                                            AddStatusLine(Translate("M1060"));
                                            test = false;
                                            abbruch = true;
                                        }
                                    } while (test);
                                    if (!abbruch) {
                                        AddStatusLine(Translate("Solldaten") + " " + Translate("vom Server") + " " + Translate("empfangen"));
                                        // Anlage speichern
                                        Anlage = TAnlage.Create();
                                        try {
                                            Anlage.LeseAnlageDaten(2);
                                        }
                                        catch (Exception& e) {
                                            AppException1.WriteError("Fehler beim Lesen der Solldaten, " + e.Message);
                                        }
                                        PufferKuerzen(54);
                                    }
                                }
                                break;
                            case 10: // RTC-Daten vom Server erhalten
                                if (puffer.anz < 16) {
                                    abbruch = true;
                                }
                                else {
                                    // neue RTC-Daten einlesen
                                    test = true;
                                    do {
                                        PufferKuerzen(1);
                                        if (puffer.anz > 15) {
                                            if (puffer.b[0] == 10) {
                                                test = false;
                                            }
                                            else {
                                                AddStatusLine(Translate("M1070"));
                                                test = false;
                                                abbruch = true;
                                            }
                                        }
                                        else {
                                            AddStatusLine(Translate("M1070"));
                                            test = false;
                                            abbruch = true;
                                        }
                                    } while (test);
                                    if (!abbruch) {
                                        AddStatusLine(Translate("RTC-Daten") + " " + Translate("vom Server") + " " + Translate("empfangen"));
                                        if (save.anzeige) {
                                            RTCZeitEinlesen(2);
                                        }
                                        PufferKuerzen(16);
                                    }
                                }
                                break;
                            case 13: // Anlagedaten vom UDP-Server erhalten
                                if (puffer.anz < 30) {
                                    abbruch = true;
                                }
                                else {
                                    // CRC-Berechnung
                                    CRC8 = TCRC8Berechnung.Create();
                                    try {
                                        for (i = 0; i < 28; i++) {
                                            CRC8.Berechne((int)puffer.b[i]);
                                        }
                                        if (CRC8.Ergebnis != (int)puffer.b[29]) {
                                            puffer.anz = 0;
                                            AddStatusLine(Translate("M1090"));
                                            AppException1.WriteError("CRC-Fehler beim Empfang");
                                            return;
                                        }
                                    }
                                    catch (Exception& e) {
                                        AppException1.WriteError("CRC-Fehler, " + e.Message);
                                    }
                                    // neue Anlagedaten einlesen
                                    test = true;
                                    do {
                                        PufferKuerzen(1);
                                        if (puffer.anz > 29) {
                                            if (puffer.b[0] == 13) {
                                                test = false;
                                            }
                                            else {
                                                AddStatusLine(Translate("M1090"));
                                                test = false;
                                                abbruch = true;
                                            }
                                        }
                                        else {
                                            AddStatusLine(Translate("M1090"));
                                            test = false;
                                            abbruch = true;
                                        }
                                    } while (test);
                                    if (!abbruch) {
                                        AddStatusLine(Translate("Anlagedaten") + " " + Translate("vom Server") + " " + Translate("empfangen"));
                                        // Anlage speichern
                                        Anlage = TAnlage.Create();
                                        try {
                                            Anlage.LeseAnlageDaten(3);
                                        }
                                        catch (Exception& e) {
                                            AppException1.WriteError("Fehler beim Lesen der Anlagedaten, " + e.Message);
                                        }
                                        PufferKuerzen(30);
                                    }
                                }
                                break;
                            case 14: // Anlagedaten mit CRC vom UDP-Server erhalten
                                if (puffer.anz < 33) {
                                    abbruch = true;
                                }
                                else {
                                    // CRC-Berechnung
                                    CRC16 = TCRC16Berechnung.Create();
                                    try {
                                        for (i = 0; i < 31; i++) {
                                            CRC16.Berechne((int)puffer.b[i]);
                                        }
                                        if ((CRC16.HiByte != (int)puffer.b[32]) || (CRC16.LoByte != (int)puffer.b[33])) {
                                            puffer.anz = 0;
                                            AddStatusLine(Translate("M1095"));
                                            AppException1.WriteError("CRC-Fehler beim Empfang");
                                            return;
                                        }
                                    }
                                    catch (Exception& e) {
                                        AppException1.WriteError("CRC-Fehler, " + e.Message);
                                    }
                                    // neue Anlagedaten einlesen
                                    test = true;
                                    do {
                                        PufferKuerzen(1);
                                        if (puffer.anz > 32) {
                                            if (puffer.b[0] == 14) {
                                                test = false;
                                            }
                                            else {
                                                AddStatusLine(Translate("M1095"));
                                                test = false;
                                                abbruch = true;
                                            }
                                        }
                                        else {
                                            AddStatusLine(Translate("M1095"));
                                            test = false;
                                            abbruch = true;
                                        }
                                    } while (test);
                                    if (!abbruch) {
                                        AddStatusLine(Translate("Anlagedaten") + " " + Translate("vom Server") + " " + Translate("empfangen"));
                                        // Anlage speichern
                                        Anlage = TAnlage.Create();
                                        try {
                                            Anlage.LeseAnlageDaten(4);
                                        }
                                        catch (Exception& e) {
                                            AppException1.WriteError("Fehler beim Lesen der Anlagedaten, " + e.Message);
                                        }
                                        PufferKuerzen(34);
                                    }
                                }
                                break;
                            case 15: // Auftragsdaten vom UDP-Server erhalten
                                if (puffer.anz < 28) {
                                    abbruch = true;
                                }
                                else {
                                    // CRC-Berechnung
                                    CRC8 = TCRC8Berechnung.Create();
                                    try {
                                        for (i = 0; i < 26; i++) {
                                            CRC8.Berechne((int)puffer.b[i]);
                                        }
                                        if (CRC8.Ergebnis != (int)puffer.b[27]) {
                                            puffer.anz = 0;
                                            AddStatusLine(Translate("M1110"));
                                            AppException1.WriteError("CRC-Fehler beim Empfang");
                                            return;
                                        }
                                    }
                                    catch (Exception& e) {
                                        AppException1.WriteError("CRC-Fehler, " + e.Message);
                                    }
                                    // neue Auftragsdaten einlesen
                                    test = true;
                                    do {
                                        PufferKuerzen(1);
                                        if (puffer.anz > 27) {
                                            if (puffer.b[0] == 15) {
                                                test = false;
                                            }
                                            else {
                                                AddStatusLine(Translate("M1110"));
                                                test = false;
                                                abbruch = true;
                                            }
                                        }
                                        else {
                                            AddStatusLine(Translate("M1110"));
                                            test = false;
                                            abbruch = true;
                                        }
                                    } while (test);
                                    if (!abbruch) {
                                        AddStatusLine(Translate("Auftragsdaten") + " " + Translate("vom Server") + " " + Translate("empfangen"));
                                        // Auftrag speichern
                                        Auftrag = TAuftrag.Create();
                                        try {
                                            Auftrag.LeseAuftragDaten(3);
                                        }
                                        catch (Exception& e) {
                                            AppException1.WriteError("Fehler beim Lesen der Auftragsdaten, " + e.Message);
                                        }
                                        PufferKuerzen(28);
                                    }
                                }
                                break;
                            case 16: // Auftragsdaten mit CRC vom UDP-Server erhalten
                                if (puffer.anz < 31) {
                                    abbruch = true;
                                }
                                else {
                                    // CRC-Berechnung
                                    CRC16 = TCRC16Berechnung.Create();
                                    try {
                                        for (i = 0; i < 29; i++) {
                                            CRC16.Berechne((int)puffer.b[i]);
                                        }
                                        if ((CRC16.HiByte != (int)puffer.b[29]) || (CRC16.LoByte != (int)puffer.b[30])) {
                                            puffer.anz = 0;
                                            AddStatusLine(Translate("M1115"));
                                            AppException1.WriteError("CRC-Fehler beim Empfang");
                                            return;
                                        }
                                    }
                                    catch (Exception& e) {
                                        AppException1.WriteError("CRC-Fehler, " + e.Message);
                                    }
                                    // neue Auftragsdaten einlesen
                                    test = true;
                                    do {
                                        PufferKuerzen(1);
                                        if (puffer.anz > 30) {
                                            if (puffer.b[0] == 16) {
                                                test = false;
                                            }
                                            else {
                                                AddStatusLine(Translate("M1115"));
                                                test = false;
                                                abbruch = true;
                                            }
                                        }
                                        else {
                                            AddStatusLine(Translate("M1115"));
                                            test = false;
                                            abbruch = true;
                                        }
                                    } while (test);
                                    if (!abbruch) {
                                        AddStatusLine(Translate("Auftragsdaten") + " " + Translate("vom Server") + " " + Translate("empfangen"));
                                        // Auftrag speichern
                                        Auftrag = TAuftrag.Create();
                                        try {
                                            Auftrag.LeseAuftragDaten(4);
                                        }
                                        catch (Exception& e) {
                                            AppException1.WriteError("Fehler beim Lesen der Auftragsdaten, " + e.Message);
                                        }
                                        PufferKuerzen(31);
                                    }
                                }
                                break;
                            default:
                                AddStatusLine(Translate("M1195"));
                                abbruch = true;
                                break;
                        }
                        if (abbruch) {
                            AddStatusLine(Translate("M1190"));
                        }
                    }
                }
            }
        }
        // Dekodieren
        catch (Exception& e) {
            AppException1.WriteError("Fehler, " + e.Message);
        }
    } 
    }
}
