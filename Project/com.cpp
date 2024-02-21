#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <libserialport.h>
#include <cmath>


void BerechneT(unsigned char t_lsb, unsigned char t_msb, unsigned char t_rem, unsigned char t_per, double& nTemperatur, bool& bOK) {
    double nMerk = nTemperatur;
    bOK = true;

    if (t_per == 0) {
        bOK = false;
    } else {
        if (t_msb == 0) {
            nTemperatur = t_lsb / 2.0;
            if (t_per != 0) {
                nTemperatur = nTemperatur - 0.25 + (t_per - t_rem) / static_cast<double>(t_per);
            }
        } else {
            nTemperatur = -((~t_lsb) + 1) / 2.0;
            if (t_per != 0) {
                nTemperatur = nTemperatur - 0.25 + (t_per - t_rem) / static_cast<double>(t_per);
            }
        }
        nTemperatur = round(nTemperatur * 10) / 10.0;
    }

    if (nTemperatur < -10 || nTemperatur > 110) {
        nTemperatur = nMerk;
        bOK = false;
    }
}

void SendString(const std::string& s) {
    if (aufsendung)
        return;

    aufsendung = true;


    std::string ss20;
    for (size_t i = 0; i < s.length(); ++i)
        ss20 += ' ' + std::to_string(static_cast<int>(s[i]));

    // Display to user if frmEmpfang is active and menu item is checked
    if (frmEmpfang != nullptr && mnuEmpfangspufferanzeigen.Checked) {
        try {
            if (Empfangspuffer != nullptr)
                Empfangspuffer->Save("S" + ss20);
            frmEmpfang->Memo1->Lines->Add("S" + ss20);
        } catch (EOutOfResources&) {
            frmEmpfang->Memo1->Lines->Clear();
            frmEmpfang->Memo1->Lines->Add("S" + ss20);
        } catch (...) {
            AppException1.WriteError("Fehler beim Schreiben des Empfangspuffers");
        }
    }

    if (s.length() > 0) {
        std::string ss = s;

        // Send data through serial port
        struct sp_port *port;
        sp_get_port_by_name("/dev/ttyUSB0", &port); // Change port name as per your requirement
        sp_open(port, SP_MODE_READ_WRITE);
        sp_blocking_write(port, ss.c_str(), ss.size(), 0);

        sp_close(port);
        sp_free_port(port);
    }

    aufsendung = false; // Reset aufsendung after sending is done
}