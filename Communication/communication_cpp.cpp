void TfrmMain::Receive(string Str, string sFromIP) {
    int i;
    double t;
    bool abbruch, aender_leitung, aender_gb, aender_mb, test, merk, bolo;
    char ch;
    string s, sMessage;
    TAnlage Anlage;
    byte protokoll;

    connect_counter = 20;
    if (save.com.ok && Str != "") {
        tbConnect.Visible = true;
        try {
            try {
#ifdef control
                control_haupt.Panel1.Caption = Translate("M1290");
                control_haupt.Panel1.update();
#endif
                will_senden = false;
                s = "";
                if (Str != "") {
                    for (i = 1; i <= Str.length(); i++) {
                        if (puffer.anz >= sizeof(puffer.b)) {
                            puffer.anz = 0;
                            merk = save.anzeige;
                            save.anzeige = false;
                            AddStatusLine(Translate("M1010"));
                            save.anzeige = merk;
                        }
                        ch = Str[i];
                        puffer.b[puffer.anz] = ch;
                        puffer.anz++;
                        s += " " + to_string(static_cast<int>(ch));
                    }
                }
                if (frmEmpfang != nullptr && mnuEmpfangspuffernutzen.Checked) {
                    try {
                        if (Empfangspuffer != nullptr) {
                            Empfangspuffer.Save("E" + s);
                        }
                        frmEmpfang.Memo1.Lines.Add("E" + s);
                    }
                    catch (EOutOfResources) {
                        frmEmpfang.Memo1.Lines.Clear();
                        frmEmpfang.Memo1.Lines.Add("E" + s);
                    }
                    catch (...) {
                        AppException1.WriteError("Fehler beim Schreiben des Empfangspuffers");
                        throw;
                    }
                }
                abbruch = false;
                do {
                    if (puffer.anz > 0) {
                        switch (static_cast<int>(puffer.b[0])) {
                        case 6: {
                            if (puffer.anz < 1) {
                                puffer.anz = 0;
                            }
                            else {
                                Send2Client(1);
                                do {
                                    PufferKuerzen(1);
                                } while (puffer.anz != 0 && static_cast<int>(puffer.b[0]) != 6);
                                SendeStatus();
                            }
                            break;
                        }
                        case 7:
                        case 5: {
                            sMessage = "";
                            i = 1;
                            while (i < puffer.anz && puffer.b[i] != '\0') {
                                sMessage += puffer.b[i];
                                i++;
                            }
                            if (puffer.b[0] == 7) {
                                AddStatusLine(Translate("Message") + " \"" + sMessage + "\" " + Translate("vom Client") + " " + sFromIP + " " + Translate("empfangen"));
                            }
                            else if (puffer.b[0] == 5) {
                                int nIndex = -1;
                                for (i = 0; i < save.com.lstClient.Count; i++) {
                                    BraxoniaClient = dynamic_cast<TBraxoniaClient*>(save.com.lstClient.Items[i]);
                                    if (BraxoniaClient->IP == sFromIP || BraxoniaClient->DNSName == sFromIP) {
                                        nIndex = i;
                                        break;
                                    }
                                }
                                if (nIndex != -1) {
                                    BraxoniaClient = dynamic_cast<TBraxoniaClient*>(save.com.lstClient.Items[nIndex]);
                                    BraxoniaClient->Update();
                                }
                            }
                            PufferKuerzen(sMessage.length() + 2);
                            break;
                        }
                        case 9: {
                            if (puffer.anz < 54) {
                                abbruch = true;
                            }
                            else {
                                // CRC-Berechnung
                                CRC8 = new TCRC8Berechnung();
                                try {
                                    for (i = 0; i < 52; i++) {
                                        CRC8->Berechne(static_cast<int>(puffer.b[i]));
                                    }
                                    if (CRC8->Ergebnis != static_cast<int>(puffer.b[53])) {
                                        puffer.anz = 0;
                                        AddStatusLine(Translate("M1060"));
                                        AppException1.WriteError("CRC-Fehler beim Empfang");
                                        return;
                                    }
                                }
                                catch (Exception& E) {
                                    AppException1.WriteError("CRC-Fehler, " + string(E.Message));
                                }
                                finally {
                                    delete CRC8;
                                }
                                tmpPara = save.para.GetKopie();
                                try {
                                    // 1. Byte Maischbottich 1
                                    tmpPara.MaischBottich1 = From5toI(&puffer.b[1], 2);
                                    // 3. Byte Maischbottich 2
                                    tmpPara.MaischBottich2 = From5toI(&puffer.b[3], 2);
                                    // 5. Byte Kochbottich
                                    tmpPara.KochBottich = From5toI(&puffer.b[5], 2);
                                    // 7. Byte Temperaturverzögerung
                                    tmpPara.Temperaturverzoegerung = From5toI(&puffer.b[7], 2);
                                    // 9. Byte Warmhalten
                                    tmpPara.Warmhalten = From5toI(&puffer.b[9], 2);
                                    // 11. Byte Abkühlen
                                    tmpPara.Abkuehlen = From5toI(&puffer.b[11], 2);
                                    // 13. Byte Abkühlmenge
                                    tmpPara.Abkuehlmenge = From5toI(&puffer.b[13], 3);
                                    // 16. Byte Würzekochtemperatur
                                    tmpPara.Wuerzekochtemperatur = From5toI(&puffer.b[16], 2);
                                    // 18. Byte Sudhausmenge
                                    tmpPara.Sudhausmenge = From5toI(&puffer.b[18], 3);
                                    // 21. Byte Kochzeit
                                    tmpPara.Kochzeit = From5toI(&puffer.b[21], 3);
                                    // 24. Byte Sudhausmenge laut Fühler
                                    tmpPara.SudhausmengeFuehler = From5toI(&puffer.b[24], 3);
                                    // 27. Byte Sudhaus-zeit
                                    tmpPara.Sudhauszeit = From5toI(&puffer.b[27], 2);
                                    // 29. Byte Filter
                                    tmpPara.Filter = From5toI(&puffer.b[29], 2);
                                    // 31. Byte Ausschlagmenge
                                    tmpPara.Ausschlagmenge = From5toI(&puffer.b[31], 3);
                                    // 34. Byte Filtrieren
                                    tmpPara.Filtrieren = From5toI(&puffer.b[34], 2);
                                    // 36. Byte Pausenmischer
                                    tmpPara.Pausenmischer = From5toI(&puffer.b[36], 2);
                                    // 38. Byte Dampfzeit
                                    tmpPara.Dampfzeit = From5toI(&puffer.b[38], 2);
                                    // 40. Byte Dampfdruck
                                    tmpPara.Dampfdruck = From5toI(&puffer.b[40], 2);
                                    // 42. Byte Hopfengabe
                                    tmpPara.Hopfengabe = From5toI(&puffer.b[42], 3);
                                    // 45. Byte Dampfmengen
                                    tmpPara.Dampfmengen = From5toI(&puffer.b[45], 3);
                                    // 48. Byte Kegel
                                    tmpPara.Kegel = From5toI(&puffer.b[48], 2);
                                    // 50. Byte KEG-Ablauf
                                    tmpPara.KEGAb = From5toI(&puffer.b[50], 2);
                                    // 52. Byte Zapfenzeit
                                    tmpPara.Zapfenzeit = From5toI(&puffer.b[52], 2);
                                    tmpPara.SudhausTemperatur = to_string(tmpPara.KochBottich);
                                    // 54. Byte Crc
                                    SaveAktualisieren();
                                }
                                catch (...) {
                                    AppException1.WriteError("Fehler beim Empfang der Parameter");
                                    // SaveDaten;
                                }
                                delete tmpPara;
                            }
                            break;
                        }
                        default: {
                            if (puffer.anz > 1 && static_cast<int>(puffer.b[1]) == 6) {
                                SendeStatus();
                            }
                            abbruch = true;
                            break;
                        }
                        }
                    }
                    else {
                        abbruch = true; 
                    }
                } while (!abbruch);
            }
            catch (Exception& E) {
                AppException1.WriteError("Fehler beim Empfang, " + string(E.Message));
            }
            finally {
                if (save.anzeige) {
#ifdef control
                    control_haupt.Panel1.Caption = Translate("M1300");
                    control_haupt.Panel1.update();
#endif
                }
            }
        }
        catch (Exception& E) {
            AppException1.WriteError("Fehler beim Empfang, " + string(E.Message));
        }
    }
}
