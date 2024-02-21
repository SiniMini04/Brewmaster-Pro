#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Color_Chooser.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Check_Button.H>
#include "main.h"
#include "backend.h"
#include "brauschritte.h"
#include "global.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <FL/Fl_File_Chooser.H>

/*

Backend Functions

*/

nlohmann::json default_values;
nlohmann::json recipe_values;

Fl_Int_Input *rast1_time;
Fl_Int_Input *rast1_temp;
Fl_Int_Input *rast2_time;
Fl_Int_Input *rast2_temp;
Fl_Int_Input *rast3_time;
Fl_Int_Input *rast3_temp;

/*

Frontend Functions

*/

void putzen(Fl_Widget *, void *v)
{

    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *hefe = data->window;
    int number = data->number;
    delete data;

    hefe->hide();

    std::cout << "Number: " << number << std::endl;

    Fl_Window *info_window = new Fl_Window(600, 150, "Putzen");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Vergessen sie nicht die Anlage zu putzen!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");

    if (number == 1)
    {
        soll_temp_maischbottich1 = 0.0;
        update_soll_temperature();
    }
    else if (number == 2)
    {
        soll_temp_maischbottich2 = 0.0;
        update_soll_temperature2();
    }

    ok_button->callback(close_cb, info_window);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void hefe(Fl_Widget *, void *v)
{

    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *menge_temperatur_messen = data->window;
    int number = data->number;
    delete data;

    menge_temperatur_messen->hide();

    Fl_Window *info_window = new Fl_Window(600, 150, "Hefe dazu geben");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Geben sie nun die Hefe hinzu!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(putzen, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void menge_temperatur_messen(Fl_Widget *, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *stammwurze_messen = data->window;
    int number = data->number;
    delete data;

    stammwurze_messen->hide();

    Fl_Window *time_window = new Fl_Window(500, 200, "Messung Menge und Temperatur");

    int center_x = time_window->w() / 2;
    int center_y = time_window->h() / 2;

    Fl_Box *instruction_box = new Fl_Box(center_x - 100, center_y - 80, 200, 20, "Geben sie die Biermengenmenge und die Temperatur ein:");
    Fl_Int_Input *time_input = new Fl_Int_Input(center_x - 50, center_y - 40, 100, 30, "Menge:");
    Fl_Box *time_unit = new Fl_Box(center_x + 55, center_y - 40, 30, 30, "L");
    Fl_Int_Input *temp_input = new Fl_Int_Input(center_x - 50, center_y, 100, 30, "Temp:");
    Fl_Box *temp_unit = new Fl_Box(center_x + 55, center_y, 30, 30, "°C");

    Fl_Button *ok_button = new Fl_Button(center_x + 20, center_y + 50, 100, 25, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{time_window, number};
    ok_button->callback(hefe, newdata);

    Fl_Button *cancel_button = new Fl_Button(center_x - 120, center_y + 50, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, time_window);

    time_window->end();
    time_window->set_modal();
    time_window->show();
}

void stammwurze_messen(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *time_window = new Fl_Window(400, 200, "Messung Stamwürze");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Geben sie die Stammwürzmenge ein:");
    Fl_Int_Input *time_input = new Fl_Int_Input(150, 60, 100, 30, "Stammwürze:");
    Fl_Box *time_unit = new Fl_Box(255, 60, 30, 30, "°P");

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{time_window, number};
    ok_button->callback(menge_temperatur_messen, newdata);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, time_window);

    time_window->end();
    time_window->set_modal();
    time_window->show();
}

void timer_stammwuerze_messung(Fl_Widget *, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *ausschlagen = data->window;
    int number = data->number;
    delete data;

    ausschlagen->hide();

    Fl_Window *time_window = new Fl_Window(400, 200, "Timer Stammwürze Messung");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Setzen sie den Timer für das Messen der Stammwürze:");
    Fl_Int_Input *time_input = new Fl_Int_Input(150, 60, 100, 30, "Zeit:");
    time_input->value(std::to_string(static_cast<int>(default_values["stammwuerze-messung-time"])).c_str());
    Fl_Box *time_unit = new Fl_Box(255, 60, 30, 30, "min.");

    static std::function<void(Fl_Widget *, void *)> callback = [number, time_input](Fl_Widget *w, void *)
    {
        Fl_Window *window = static_cast<Fl_Window *>(w->window());
        int time = std::stoi(time_input->value());
        timer_stammwuerze_messung_backend(window, time, number);
    };

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    ok_button->callback([](Fl_Widget *w, void *v)
                        { callback(w, v); },
                        nullptr);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback([](Fl_Widget *w, void *v)
                            { w->window()->hide(); },
                            nullptr);

    time_window->end();
    time_window->set_modal();
    time_window->show();
}

void ausschlagen(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Ausschlagen");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Öffnen sie das Ventil zum Gärtank!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(timer_stammwuerze_messung, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void tank_angebeb(Fl_Widget *, void *v)
{

    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *wirlen = data->window;
    int number = data->number;
    delete data;

    wirlen->hide();

    Fl_Window *info_window = new Fl_Window(600, 150, "Tank angeben");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Hier sollten Tanks zum auswählen sein!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(close_tank_angebeb, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void wirlen(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Wirlen");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Das Wirlen wurde beendet.");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(tank_angebeb, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void timer_wirlen(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *time_window = new Fl_Window(400, 200, "Timer Wirlen");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Setzen sie den Timer für das Wirlen:");
    Fl_Int_Input *time_input = new Fl_Int_Input(150, 60, 100, 30, "Zeit:");
    time_input->value(std::to_string(static_cast<int>(default_values["wirlen-time"])).c_str());
    Fl_Box *time_unit = new Fl_Box(255, 60, 30, 30, "min.");

    static std::function<void(Fl_Widget *, void *)> callback = [number, time_input](Fl_Widget *w, void *)
    {
        Fl_Window *window = static_cast<Fl_Window *>(w->window());
        int time = std::stoi(time_input->value());
        timer_wirlen_backend(window, time, number);
    };

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    ok_button->callback([](Fl_Widget *w, void *v)
                        { callback(w, v); },
                        nullptr);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, time_window);

    time_window->end();
    time_window->set_modal();
    time_window->show();
}

void umpumpen_in_maischbottich_fuer_wirlen(Fl_Widget *, void *v)
{

    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *wuerzekochen_mit_hopfengabe_2 = data->window;
    int number = data->number;
    delete data;

    wuerzekochen_mit_hopfengabe_2->hide();

    Fl_Window *info_window = new Fl_Window(600, 150, "Umpumpen in Läuterbottich für Wirlen");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Öffnen sie die Ventiele zum Maischbottich!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(close_umpumpen_in_maischbottich_fuer_wirlen, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void wuerzekochen_mit_hopfengabe_2(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Würzekochen mit Hopfengabe 2");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Geben sie nun den zweiten Teil Hopfen hinzu!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(umpumpen_in_maischbottich_fuer_wirlen, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void wuerzekochen_mit_hopfengabe(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Würzekochen mit Hopfengabe");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Geben sie nun den ersten Teil Hopfen hinzu!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(wuerzekochen_mit_hopfengabe2_backend, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void vollwurze_messen(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *time_window = new Fl_Window(400, 200, "Messung Vollwürze");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Geben sie die Vollwürzemenge ein:");
    Fl_Int_Input *time_input = new Fl_Int_Input(150, 60, 100, 30, "Vollwürze:");
    Fl_Box *time_unit = new Fl_Box(255, 60, 30, 30, "°P");

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{time_window, number};
    ok_button->callback(wuerzekochen_mit_hopfengabe1_backend, newdata);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, time_window);

    time_window->end();
    time_window->set_modal();
    time_window->show();
}

struct Inputs
{
    Fl_Int_Input *time_input;
    Fl_Int_Input *time_input2;
    Fl_Int_Input *temp_input;
};

void temp_zeit_einstellen_wuerzekochen(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 190, "Zeit und Temperatur einstellen für Würzekochen mit Hopfengabe");

    Fl_Box *info_box = new Fl_Box(20, 20, 560, 30, "Geben sie nun die Zeit und Temperatur für Würzekochen mit Hopfengabe ein!");
    Fl_Int_Input *time_input = new Fl_Int_Input(150, 60, 100, 30, "Zeit 1. Teil:");
    time_input->value(std::to_string(static_cast<int>(default_values["wuerzekochen-time1"])).c_str());
    Fl_Box *time_unit = new Fl_Box(255, 60, 30, 30, "min.");

    Fl_Int_Input *time_input2 = new Fl_Int_Input(150, 100, 100, 30, "Zeit 2. Teil:");
    time_input2->value(std::to_string(static_cast<int>(default_values["wuerzekochen-time2"])).c_str());
    Fl_Box *time_unit2 = new Fl_Box(255, 100, 30, 30, "min.");

    Fl_Int_Input *temp_input = new Fl_Int_Input(410, 80, 100, 30, "Temp:");
    temp_input->value(std::to_string(static_cast<int>(default_values["wuerzekochen-temp"])).c_str());
    Fl_Box *temp_unit = new Fl_Box(515, 80, 30, 30, "°C");

    static std::function<void(Fl_Widget *, void *)> callback = [number, time_input, time_input2, temp_input](Fl_Widget *w, void *)
    {
        Fl_Window *window = static_cast<Fl_Window *>(w->window());
        int time1 = std::stoi(time_input->value());
        int time2 = std::stoi(time_input2->value());
        int temp = std::stoi(temp_input->value());
        temp_zeit_einstellen_wuerzekochen_backend(window, time1, time2, temp, number);
    };

    Fl_Button *ok_button = new Fl_Button(250, 150, 100, 30, "OK");
    ok_button->callback([](Fl_Widget *w, void *v)
                        { callback(w, v); },
                        nullptr);

    Fl_Button *cancel_button = new Fl_Button(360, 150, 100, 30, "Abbrechen");
    cancel_button->callback(cancel_cb, info_window);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void check_cb(Fl_Widget *w, void *v)
{
    Fl_Check_Button *check = (Fl_Check_Button *)w;
    Fl_Button *ok_button = (Fl_Button *)v;
    if (check->value() == 1)
    {
        ok_button->activate();
    }
    else
    {
        ok_button->deactivate();
    }
}

void heizstangen_bedekt(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Sind die Heizstangen bedeckt?");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Sind die Heizstangen mit Wasser bedeckt?");

    int checkbox_width = 100;
    int checkbox_height = 30;
    int checkbox_x = (info_window->w() - checkbox_width) / 2;
    int checkbox_y = (info_window->h() - checkbox_height) / 2;

    Fl_Check_Button *check = new Fl_Check_Button(checkbox_x, checkbox_y, checkbox_width, checkbox_height, "Ja");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(close_heizstangen_bedekt_backend, newdata);
    ok_button->deactivate();

    check->callback(check_cb, ok_button);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void anschwanzen(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Anschwänzen");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Geben sie nochmals 12L Wasser hinzu!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataN *newdata = new CallbackDataN{number};
    ok_button->callback([](Fl_Widget *w, void *v)
                        {
                            CallbackDataN* data = static_cast<CallbackDataN*>(v);
    int number = data->number;
        Fl_Window* window = static_cast<Fl_Window*>(w->window());
        int time = std::stoi(std::to_string(static_cast<int>(default_values["heizstangen-time"])).c_str());
        heizstangen_bedekt_backend(window, time, number); },
                        newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void vorwurze_messen(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *time_window = new Fl_Window(400, 200, "Messung Vorwürze");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Geben sie die Vorwürzemenge ein:");
    Fl_Int_Input *time_input = new Fl_Int_Input(150, 60, 100, 30, "Vorwürze:");
    Fl_Box *time_unit = new Fl_Box(255, 60, 30, 30, "°P");

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{time_window, number};
    ok_button->callback(vorwurze_messen_backend, newdata);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, time_window);

    time_window->end();
    time_window->set_modal();
    time_window->show();
}

void timer_vorwurze(Fl_Widget *, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *umpumpen_maischbottich_nach_lautern = data->window;
    int number = data->number;
    delete data;

    umpumpen_maischbottich_nach_lautern->hide();

    Fl_Window *time_window = new Fl_Window(400, 200, "Timer Messung Vorwürze");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Timer für Vorwürzemessung einstellen:");
    Fl_Int_Input *time_input = new Fl_Int_Input(150, 60, 100, 30, "Zeit:");
    time_input->value(std::to_string(static_cast<int>(default_values["vorwuerzmessung-time"])).c_str());
    Fl_Box *time_unit = new Fl_Box(255, 60, 30, 30, "min.");

    static std::function<void(Fl_Widget *, void *)> callback = [time_input, number](Fl_Widget *w, void *)
    {
        Fl_Window *window = static_cast<Fl_Window *>(w->window());
        int time = std::stoi(time_input->value());
        timer_vorwurze_backend(window, time, number);
    };

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    ok_button->callback([](Fl_Widget *w, void *v)
                        { callback(w, v); },
                        nullptr);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, time_window);

    time_window->end();
    time_window->set_modal();
    time_window->show();
}

void umpumpen_maischbottich_nach_lautern(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Umpumpen in Maischbottich nach Läutern");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Öffenen sie die Ventiele zum Maischbottich!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(timer_vorwurze, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void lauterzeit_einstellen_cb(Fl_Widget *, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *umpumpen_lauterbottich = data->window;
    int number = data->number;
    delete data;

    umpumpen_lauterbottich->hide();

    Fl_Window *time_window = new Fl_Window(400, 200, "Läuterzeit einstellen");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Läuterzeit einstellen:");
    Fl_Int_Input *time_input = new Fl_Int_Input(150, 60, 100, 30, "Zeit:");
    time_input->value(std::to_string(static_cast<int>(default_values["ablauter-time"])).c_str());
    Fl_Box *time_unit = new Fl_Box(255, 60, 30, 30, "min");

    static std::function<void(Fl_Widget *, void *)> callback = [time_input, number](Fl_Widget *w, void *)
    {
        Fl_Window *window = static_cast<Fl_Window *>(w->window());
        int time = std::stoi(time_input->value());
        lauterzeit_einstellen_cb_backend(window, time, number);
    };

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    ok_button->callback([](Fl_Widget *w, void *v)
                        { callback(w, v); },
                        nullptr);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, time_window);

    time_window->end();
    time_window->set_modal();
    time_window->show();
}

void umpumpen_lauterbottich(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Umpumpen in Läuterbottich");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Öffenen sie die Ventiele zum Läuterbottich!");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(lauterzeit_einstellen_cb, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void ablautertemperatur_einstellen_cb(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *temp_window = new Fl_Window(400, 200, "Abläutertemperatur einstellen");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Abläutertemperatur einstellen:");
    Fl_Int_Input *temp_input = new Fl_Int_Input(150, 60, 100, 30, "Temp:");
    temp_input->value(std::to_string(static_cast<int>(default_values["ablauter-temp"])).c_str());
    Fl_Box *temp_unit = new Fl_Box(255, 60, 30, 30, "°C");

    static std::function<void(Fl_Widget *, void *)> callback = [temp_input, number](Fl_Widget *w, void *)
    {
        Fl_Window *window = static_cast<Fl_Window *>(w->window());
        int temp = std::stoi(temp_input->value());
        ablautertemperatur_einstellen_cb_backend(window, temp, number);
    };

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    ok_button->callback([](Fl_Widget *w, void *v)
                        { callback(w, v); },
                        nullptr);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, temp_window);

    temp_window->end();
    temp_window->set_modal();
    temp_window->show();
}

void rast3_fertig_cb(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Rast 3 fertig");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Die dritte Rast wurde beendet.");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(close_rast3, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void rast2_fertig_cb(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Rast 2 fertig");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Die zweite Rast wurde beendet. OK drücken um die nächste Rast zu starten.");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(rast3_backend, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void rast1_fertig_cb(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Rast 1 fertig");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Die erste Rast wurde beendet. OK drücken um die nächste Rast zu starten.");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(rast2_backend, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void ok_button_callback(Fl_Widget *, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;

    int rast1_time_int = std::stoi(rast1_time->value());
    int rast1_temp_int = std::stoi(rast1_temp->value());
    int rast2_time_int = std::stoi(rast2_time->value());
    int rast2_temp_int = std::stoi(rast2_temp->value());
    int rast3_time_int = std::stoi(rast3_time->value());
    int rast3_temp_int = std::stoi(rast3_temp->value());

    rasten_einstellen_cb_backend(window, rast1_temp_int, rast1_time_int, rast2_temp_int, rast2_time_int, rast3_temp_int, rast3_time_int, number);
}

void rasten_einstellen_cb(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    int window_width = 600;
    int window_height = 350; // y-coordinate of the last element + height of the last element + some padding

    Fl_Window *rasten_window = new Fl_Window(window_width, window_height, "Rasten Zeiten und Temperaturen");

    Fl_Box *info_box = new Fl_Box(0, 40, window_width, 30, "Bitte geben Sie die Zeiten und Temperaturen für die Rasten ein.");
    info_box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

    Fl_Box *rast1_label = new Fl_Box(10, 100, 100, 30, "Rast 1:");
    rast1_time = new Fl_Int_Input(180, 100, 100, 30, "Zeit:");
    rast1_time->value(std::to_string(static_cast<int>(recipe_values["rast1"]["time"])).c_str());
    Fl_Box *rast1_time_unit = new Fl_Box(280, 100, 50, 30, "min.");
    rast1_temp = new Fl_Int_Input(410, 100, 100, 30, "Temp:");
    rast1_temp->value(std::to_string(static_cast<int>(recipe_values["rast1"]["temp"])).c_str());
    Fl_Box *rast1_temp_unit = new Fl_Box(510, 100, 30, 30, "°C");

    Fl_Box *rast2_label = new Fl_Box(10, 150, 100, 30, "Rast 2:");
    rast2_time = new Fl_Int_Input(180, 150, 100, 30, "Zeit:");
    rast2_time->value(std::to_string(static_cast<int>(recipe_values["rast2"]["time"])).c_str());
    Fl_Box *rast2_time_unit = new Fl_Box(280, 150, 50, 30, "min.");
    rast2_temp = new Fl_Int_Input(410, 150, 100, 30, "Temp:");
    rast2_temp->value(std::to_string(static_cast<int>(recipe_values["rast2"]["temp"])).c_str());
    Fl_Box *rast2_temp_unit = new Fl_Box(510, 150, 30, 30, "°C");

    Fl_Box *rast3_label = new Fl_Box(10, 200, 100, 30, "Rast 3:");
    rast3_time = new Fl_Int_Input(180, 200, 100, 30, "Zeit:");
    rast3_time->value(std::to_string(static_cast<int>(recipe_values["rast3"]["time"])).c_str());
    Fl_Box *rast3_time_unit = new Fl_Box(280, 200, 50, 30, "min.");
    rast3_temp = new Fl_Int_Input(410, 200, 100, 30, "Temp:");
    rast3_temp->value(std::to_string(static_cast<int>(recipe_values["rast3"]["temp"])).c_str());
    Fl_Box *rast3_temp_unit = new Fl_Box(510, 200, 30, 30, "°C");

    int button_width = 100;
    int button_spacing = (window_width - 2 * button_width) / 3;

    Fl_Button *ok_button = new Fl_Button(2 * button_spacing + button_width, 300, button_width, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{rasten_window, number};
    ok_button->callback(ok_button_callback, newdata);

    Fl_Button *cancel_button = new Fl_Button(button_spacing, 300, button_width, 30, "Abbrechen");
    cancel_button->callback(cancel_cb, rasten_window);

    rasten_window->end();
    rasten_window->set_modal();
    rasten_window->show();
}

void einmaischtempertatur_erreicht_cb(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *info_window = new Fl_Window(600, 150, "Einmaischtemperatur erreicht");

    Fl_Box *info_box = new Fl_Box(20, 20, info_window->w() - 40, 60, "Die Einmaischtemperatur wurde erreicht. Fügen sie nun das Malz hinzu.");

    Fl_Button *ok_button = new Fl_Button(info_window->w() / 2 - 40, info_window->h() - 40, 80, 30, "OK");
    CallbackDataBN *newdata = new CallbackDataBN{info_window, number};
    ok_button->callback(einmaischtempertatur_erreicht_cb_backend, newdata);

    info_window->end();
    info_window->set_modal();
    info_window->show();
}

void change_einmaischtemperature_cb(Fl_Widget *, void *v)
{
    CallbackDataN *data = static_cast<CallbackDataN *>(v);
    int number = data->number;
    delete data;

    Fl_Window *temp_window = new Fl_Window(400, 200, "Einmaischtemperatur einstellen");

    Fl_Box *instruction_box = new Fl_Box(100, 20, 200, 20, "Einmaischtemperatur einstellen:");
    Fl_Int_Input *temp_input = new Fl_Int_Input(150, 60, 100, 30, "Temp:");
    temp_input->value(std::to_string(static_cast<int>(recipe_values["einmaisch-temp"])).c_str());
    Fl_Box *temp_unit = new Fl_Box(255, 60, 30, 30, "°C");

    static std::function<void(Fl_Widget *, void *)> callback = [number](Fl_Widget *w, void *v)
    {
        Fl_Window *window = static_cast<Fl_Window *>(w->window());
        Fl_Int_Input *temp_input = static_cast<Fl_Int_Input *>(v);
        int temp = std::stoi(temp_input->value());
        change_einmaischtemperature_cb_backend(window, temp, number);
    };

    Fl_Button *ok_button = new Fl_Button(220, 120, 100, 25, "OK");
    ok_button->callback([](Fl_Widget *w, void *v)
                        { callback(w, v); },
                        temp_input);

    Fl_Button *cancel_button = new Fl_Button(80, 120, 100, 25, "Abbrechen");
    cancel_button->callback(cancel_cb, temp_window);

    temp_window->end();
    temp_window->set_modal();
    temp_window->show();
}

void brew_step_selected_cb(Fl_Widget *w, void *v)
{
    CallbackDataBBN *data = static_cast<CallbackDataBBN *>(v);
    Fl_Browser *browser = data->browser;
    int number = data->number;
    delete data;

    CallbackDataN *newdata = new CallbackDataN{number};

    std::string selected_step = "Einmaischen";

    if (browser->value() != 0)
    {
        selected_step = browser->text(browser->value());
    }

    if (selected_step == "Einmaischen")
    {
        change_einmaischtemperature_cb(browser, newdata);
    }
    else if (selected_step == "Rast 1")
    {
        rasten_einstellen_cb(browser, newdata);
    }
    else if (selected_step == "Rast 2")
    {
        rast1_fertig_cb(browser, newdata);
    }
    else if (selected_step == "Rast 3")
    {
        rast2_fertig_cb(browser, newdata);
    }
    else if (selected_step == "Läutern")
    {
        ablautertemperatur_einstellen_cb(browser, newdata);
    }
    else if (selected_step == "Anschwänzen")
    {
        anschwanzen(browser, newdata);
    }
    else if (selected_step == "Würzekochen mit Hopfengabe")
    {
        temp_zeit_einstellen_wuerzekochen(browser, newdata);
    }
    else if (selected_step == "Wirlen")
    {
        timer_wirlen(browser, newdata);
    }
    else if (selected_step == "Ausschlagen")
    {
        ausschlagen(browser, newdata);
    }

    Fl_Window *window = (Fl_Window *)browser->parent();
    window->hide();
}

void choose_brauschritt_cb(Fl_Widget *w, void *v)
{
    CallbackArgs *args = static_cast<CallbackArgs *>(v);
    size_t index = args->index;
    int number = args->number;
    delete args;

    std::cout << "Number: " << number << std::endl;

    std::vector<std::string> recipe_files = get_recipe_files();
    std::vector<nlohmann::json> recipes;

    for (const auto &file : recipe_files)
    {
        std::ifstream in("recipes/" + file);
        nlohmann::json recipe;
        in >> recipe;
        recipes.push_back(recipe);
    }

    const auto &recipe = recipes[index];

    default_values = read_json_file("default_values", "default_values.json");
    recipe_values = recipe;

    Fl_Window *steps_window = new Fl_Window(300, 300, "Steps"); // Increase window width to 300
    Fl_Browser *browser = new Fl_Browser(10, 10, 280, 200);     // Adjust browser width to fit new window width
    browser->type(FL_SELECT_BROWSER);
    browser->add("Einmaischen");
    browser->add("Rast 1");
    browser->add("Rast 2");
    browser->add("Rast 3");
    browser->add("Läutern");
    browser->add("Anschwänzen");
    browser->add("Würzekochen mit Hopfengabe");
    browser->add("Wirlen");
    browser->add("Ausschlagen");

    Fl_Button *cancel_button = new Fl_Button(50, 220, 100, 25, "Abbrechen"); // Adjust x-coordinate to place at the original OK button position
    cancel_button->callback(cancel_cb, steps_window);

    Fl_Button *ok_button = new Fl_Button(160, 220, 100, 25, "OK"); // Adjust x-coordinate to place at the original Abbrechen button position
    CallbackDataBBN *data = new CallbackDataBBN{browser, number};
    ok_button->callback(brew_step_selected_cb, data);
    steps_window->end();
    steps_window->set_modal();
    steps_window->show();
}