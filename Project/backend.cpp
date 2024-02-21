#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip>
#include "global.h"
#include "main.h"
#include "brauschritte.h"
#include <chrono>
#include <thread>
#include <functional>

/*

Backend Functions

*/

float temperature_maischbottich1 = 20.0;
float soll_temp_maischbottich1 = 0.0;
float temperature_maischbottich2 = 20.0;
float soll_temp_maischbottich2 = 0.0;

int rast1_temp_global = 0;
int rast1_time_global = 0;
int rast2_temp_global = 0;
int rast2_time_global = 0;
int rast3_temp_global = 0;
int rast3_time_global = 0;

int wuerzekochen_temp_global = 0;
int wuerzekochen_time1_global = 0;
int wuerzekochen_time2_global = 0;

bool heater1_on = false;
bool mixer1_on = false;
bool heater2_on = false;
bool mixer2_on = false;
bool heating_button_oversteer = false;

 void update_temperature_cb(void* data) {
    auto* updateData = static_cast<TempUpdateData*>(data);
    
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << updateData->temperature;
    updateData->outputWidget->value(stream.str().c_str());
    delete updateData; // Clean up
}

void update_temperature1_async() {
    // Prepare the data
    auto* data = new TempUpdateData{temperature_maischbottich1, istTemp};
    // Schedule the update on the main thread
    update_temperature_cb(data);
    Fl::run();
}

void update_temperature2_async() {
    // Prepare the data
    auto* data = new TempUpdateData{temperature_maischbottich2, istTemp2};
    // Schedule the update on the main thread
    update_temperature_cb(data);
    Fl::run();
}

void update_soll_temperature()
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << soll_temp_maischbottich1;
    sollTemp->value(stream.str().c_str());
}

void update_soll_temperature2()
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << soll_temp_maischbottich2;
    sollTemp2->value(stream.str().c_str());
}

void turn_on_mixer(int number)
{
    if (number == 1)
    {
        mixer1_on = true;
    }
    else if (number == 2)
    {
        mixer2_on = true;
    }

    std::cout << "Mixer " << number << " is now on." << std::endl;
}

void turn_off_mixer(int number)
{
    if (number == 1)
    {
        mixer1_on = false;
    }
    else if (number == 2)
    {
        mixer2_on = false;
    }

    std::cout << "Mixer " << number << " is now on." << std::endl;
}

void turn_off_heating(int number)
{
    heating_button_oversteer = false;

    if (number == 1)
    {
        heater1_on = false;
    }
    else if (number == 2)
    {
        heater2_on = false;
    }

    std::cout << "Heating" << number << " is now off." << std::endl;
}

void turn_on_heating(int number)
{
    if (number == 1)
    {
        std::cout << soll_temp_maischbottich1 << std::endl;
        heater1_on = true;
        update_soll_temperature();

        if (mixer1_on == false)
        {
            turn_on_mixer(number);
        }
    }
    else if (number == 2)
    {
        std::cout << soll_temp_maischbottich2 << std::endl;
        heater2_on = true;
        update_soll_temperature2();

        if (mixer2_on == false)
        {
            turn_on_mixer(number);
        }
    }

    std::cout << "Heating " << number << " is now on." << std::endl;

    if (number == 1)
    {
        while ((temperature_maischbottich1 < soll_temp_maischbottich1 && heater1_on == true && heating_button_oversteer == false) || heating_button_oversteer == true)
        {
            temperature_maischbottich1 += 1.0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << temperature_maischbottich1 << std::endl;
            std::thread update_temp_thread([]()
                                           { update_temperature1_async(); });
            update_temp_thread.detach();
        }
    }
    else if (number == 2)
    {

        while ((temperature_maischbottich2 < soll_temp_maischbottich2 && heater2_on == true && heating_button_oversteer == false) || heating_button_oversteer == true)
        {
            temperature_maischbottich2 += 1.0;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout << temperature_maischbottich2 << std::endl;
            std::thread update_temp_thread([]()
                                           { update_temperature2_async(); });
            update_temp_thread.detach();
        }
    }

    turn_off_heating(number);
}

void timer(int time, bool keep_temp, int number)
{
    int seconds = time * 60;
    for (int i = seconds; i >= 0; --i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << i << std::endl;

        if ((temperature_maischbottich1 < soll_temp_maischbottich1 && keep_temp == true && heater1_on == false && number == 1) || (temperature_maischbottich2 < soll_temp_maischbottich2 && keep_temp == true && heater2_on == false && number == 2))
        {
            std::thread heating_thread(turn_on_heating, number);
            heating_thread.detach();
        }
    }
}

void heating_button(Fl_Widget *w, void *v)
{
    int number = reinterpret_cast<intptr_t>(v);
std::thread button_heater_thread([number](){
    if (number == 1)
    {
        if (heater1_on == false)
        {
            heating_button_oversteer = true;
            turn_on_heating(number);
        }
        else if (heater1_on == true)
        {

            turn_off_heating(number);
        }
    }
    else if (number == 2)
    {
        if (heater2_on == false)
        {
            heating_button_oversteer = true;
            turn_on_heating(number);
        }
        else if (heater2_on == true)
        {

            turn_off_heating(number);
        }
    }});
    button_heater_thread.detach();
}

void mixer_button(Fl_Widget *w, void *v)
{
    int number = reinterpret_cast<intptr_t>(v);
     std::thread mixer_thread([number](){

    if (number == 1)
    {
        if (mixer1_on == false)
        {
            turn_on_mixer(number);
        }
        else if (mixer1_on == true)
        {
            turn_off_mixer(number);
        }
    }
    else if (number == 2)
    {
        if (mixer2_on == false)
        {
            turn_on_mixer(number);
        }
        else if (mixer2_on == true)
        {
            turn_off_mixer(number);
        }
    }});
    mixer_thread.detach();
    
}

std::vector<std::string> get_recipe_files()
{
    std::vector<std::string> files;
    DIR *dirp = opendir("recipes");
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL)
    {
        std::string filename(dp->d_name);
        if (filename.length() >= 5 && filename.substr(filename.length() - 5) == ".json")
        {
            files.push_back(filename);
        }
    }
    closedir(dirp);
    return files;
}

nlohmann::json read_json_file(const std::string &folder, const std::string &filename)
{
    std::string filepath = folder + "/" + filename;
    std::ifstream file(filepath);
    nlohmann::json json_data;
    file >> json_data;
    return json_data;
}

void save_cb(Fl_Widget *w, void *v)
{
    Fl_Window *recipe_window = (Fl_Window *)v;

    // Gather the input data
    Fl_Input *name_input = (Fl_Input *)recipe_window->child(1);
    Fl_Int_Input *temp_input = (Fl_Int_Input *)recipe_window->child(3);
    Fl_Int_Input *rast1_temp_input = (Fl_Int_Input *)recipe_window->child(6);
    Fl_Int_Input *rast1_time_input = (Fl_Int_Input *)recipe_window->child(8);
    Fl_Int_Input *rast2_temp_input = (Fl_Int_Input *)recipe_window->child(11);
    Fl_Int_Input *rast2_time_input = (Fl_Int_Input *)recipe_window->child(13);
    Fl_Int_Input *rast3_temp_input = (Fl_Int_Input *)recipe_window->child(16);
    Fl_Int_Input *rast3_time_input = (Fl_Int_Input *)recipe_window->child(18);

    // Gather the input data
    std::string name = name_input->value();
    int temp = std::stoi(temp_input->value());
    int rast1_temp = std::stoi(rast1_temp_input->value());
    int rast1_time = std::stoi(rast1_time_input->value());
    int rast2_temp = std::stoi(rast2_temp_input->value());
    int rast2_time = std::stoi(rast2_time_input->value());
    int rast3_temp = std::stoi(rast3_temp_input->value());
    int rast3_time = std::stoi(rast3_time_input->value());

    // Create a JSON object
    nlohmann::json j;
    j["name"] = name;
    j["temp"] = temp;
    j["rast1"]["temp"] = rast1_temp;
    j["rast1"]["time"] = rast1_time;
    j["rast2"]["temp"] = rast2_temp;
    j["rast2"]["time"] = rast2_time;
    j["rast3"]["temp"] = rast3_temp;
    j["rast3"]["time"] = rast3_time;

    // Create a filename from the recipe name
    std::string filename = "recipes/" + name + ".json";
    // Write the JSON object to a file
    std::ofstream o(filename);
    o << j << std::endl;

    recipe_window->hide();
    load_menu();
}

/*

Backend Functions Brauschritte

*/

void close_tank_angebeb(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;
    window->hide();

    CallbackDataN *newdata = new CallbackDataN{number};
    ausschlagen(nullptr, newdata);
}

void timer_stammwuerze_messung_backend(Fl_Window *window, int time, int number)
{
    window->hide();

    std::thread heating_thread([time, number]()
                               {
      timer(time, false,number);

        CallbackDataN* newdata = new CallbackDataN{number};
        stammwurze_messen(nullptr, newdata); });

    heating_thread.detach();
}

void timer_wirlen_backend(Fl_Window *window, int time, int number)
{
    window->hide();

    std::thread heating_thread([time, number]()
                               {
      timer(time, false,number);

        CallbackDataN* newdata = new CallbackDataN{number};
        wirlen(nullptr, newdata); });

    heating_thread.detach();
}

void close_umpumpen_in_maischbottich_fuer_wirlen(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;
    window->hide();

    CallbackDataN *newdata = new CallbackDataN{number};
    timer_wirlen(nullptr, newdata);
}

void wuerzekochen_mit_hopfengabe2_backend(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;
    window->hide();

    std::thread heating_thread([number]()
                               {

    timer(wuerzekochen_time2_global, true, number);

    CallbackDataN* newdata = new CallbackDataN{number};
    wuerzekochen_mit_hopfengabe_2(nullptr, newdata); });
    heating_thread.detach();
}

void wuerzekochen_mit_hopfengabe1_backend(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;
    window->hide();

    std::thread heating_thread([number]()
                               {
                                if (number == 1)
                                {
                                    soll_temp_maischbottich1 = wuerzekochen_temp_global;
                                }
                                else if (number == 2)
                                {
                                    soll_temp_maischbottich2 = wuerzekochen_temp_global;
                                }
    turn_on_heating(number);



    timer(wuerzekochen_time1_global, true,number);

    CallbackDataN* newdata = new CallbackDataN{number};
    wuerzekochen_mit_hopfengabe(nullptr, newdata); });
    heating_thread.detach();
}

void temp_zeit_einstellen_wuerzekochen_backend(Fl_Widget *window, int time1, int time2, int temp, int number)
{

    window->hide();

    std::thread heating_thread([temp, time1, time2, number]()
                               {
                                   wuerzekochen_temp_global = temp;
                                   wuerzekochen_time1_global = time1;
                                   wuerzekochen_time2_global = time2; });
    heating_thread.detach();

    CallbackDataN *newdata = new CallbackDataN{number};
    vollwurze_messen(nullptr, newdata);
}

void close_heizstangen_bedekt_backend(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;
    window->hide();

    CallbackDataN *newdata = new CallbackDataN{number};
    temp_zeit_einstellen_wuerzekochen(nullptr, newdata);
}

void heizstangen_bedekt_backend(Fl_Window *window, int time, int number)
{
    window->hide();

    std::thread heating_thread([time, number]()
                               {
      timer(time, false,number);

        CallbackDataN* newdata = new CallbackDataN{number};
        heizstangen_bedekt(nullptr, newdata); });

    heating_thread.detach();
}

void vorwurze_messen_backend(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;
    window->hide();

    CallbackDataN *newdata = new CallbackDataN{number};
    anschwanzen(nullptr, newdata);
}

void timer_vorwurze_backend(Fl_Window *window, int time, int number)
{
    window->hide();

    std::thread heating_thread([time, number]()
                               {
      timer(time, false,number);

        CallbackDataN* newdata = new CallbackDataN{number};
        vorwurze_messen(nullptr, newdata); });

    heating_thread.detach();
}

void lauterzeit_einstellen_cb_backend(Fl_Window *window, int time, int number)
{
    window->hide();

    std::thread heating_thread([time, number]()
                               {
      timer(time, false,number);

        CallbackDataN* newdata = new CallbackDataN{number};
        umpumpen_maischbottich_nach_lautern(nullptr, newdata); });

    heating_thread.detach();
}

void ablautertemperatur_einstellen_cb_backend(Fl_Window *window, int temp, int number)
{

    window->hide();

    std::thread heating_thread([temp, number]()
                               {
                                if(number == 1)
                                {
                                    soll_temp_maischbottich1 = temp;
                                }
                                else if(number == 2)
                                {
                                    soll_temp_maischbottich2 = temp;
                                }
        turn_on_heating(number);

        CallbackDataN* newdata = new CallbackDataN{number};
        umpumpen_lauterbottich(nullptr, newdata); });

    heating_thread.detach();
}

void close_rast3(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;
    window->hide();

    CallbackDataN *newdata = new CallbackDataN{number};
    ablautertemperatur_einstellen_cb(nullptr, newdata);
}

void rast3_backend(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;
    window->hide();

    std::thread heating_thread([number]()
                               {
if(number == 1){
     soll_temp_maischbottich1 = rast3_temp_global;
}
else if(number == 2){
                                soll_temp_maischbottich2 = rast3_temp_global;}

    turn_on_heating(number);

    timer(rast3_time_global,true,number);

    CallbackDataN* newdata = new CallbackDataN{number};
    rast3_fertig_cb(nullptr, newdata); });
    heating_thread.detach();
}

void rast2_backend(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;

    window->hide();

    std::thread heating_thread([number]()
                               {

if(number == 1){soll_temp_maischbottich1 = rast2_temp_global;}
else if(number == 2){soll_temp_maischbottich2 = rast2_temp_global;}
                                

    turn_on_heating(number);

    timer(rast2_time_global,true,number);

    CallbackDataN* newdata = new CallbackDataN{number};
    rast2_fertig_cb(nullptr, newdata); });
    heating_thread.detach();
}

void rast1_backend(int number)
{
    if (number == 1)
    {
        soll_temp_maischbottich1 = rast1_temp_global;
    }
    else if (number == 2)
    {
        soll_temp_maischbottich2 = rast1_temp_global;
    }
    turn_on_heating(number);

    timer(rast1_time_global, true, number);

    CallbackDataN *newdata = new CallbackDataN{number};
    rast1_fertig_cb(nullptr, newdata);
}

void rasten_einstellen_cb_backend(Fl_Window *window, int rast1_temp, int rast1_time, int rast2_temp, int rast2_time, int rast3_temp, int rast3_time, int number)
{

    window->hide();

    std::thread heating_thread([rast1_temp, rast1_time, rast2_temp, rast2_time, rast3_temp, rast3_time, number]()
                               {

    std::cout << "Rast 1: " << std::dec << rast1_temp << "°C, " << rast1_time << "min." << std::endl;
    std::cout << "Rast 2: " << std::dec << rast2_temp << "°C, " << rast2_time << "min." << std::endl;
    std::cout << "Rast 3: " << std::dec << rast3_temp << "°C, " << rast3_time << "min." << std::endl;

    rast1_temp_global = rast1_temp;
    rast1_time_global = rast1_time;
    rast2_temp_global = rast2_temp;
    rast2_time_global = rast2_time;
    rast3_temp_global = rast3_temp;
    rast3_time_global = rast3_time;

    rast1_backend(number); });
    heating_thread.detach();
}

void einmaischtempertatur_erreicht_cb_backend(Fl_Widget *w, void *v)
{
    CallbackDataBN *data = static_cast<CallbackDataBN *>(v);
    Fl_Window *window = data->window;
    int number = data->number;
    delete data;

    window->hide();

    CallbackDataN *newdata = new CallbackDataN{number};
    rasten_einstellen_cb(nullptr, newdata);
}

void change_einmaischtemperature_cb_backend(Fl_Window *window, int temp, int number)
{
    window->hide();

    std::cout << "Number Backend: " << number << std::endl;

    std::thread heating_thread([temp, number]()
                               {
        if(number == 1)
        {
            soll_temp_maischbottich1 = temp;
        }
        else if(number == 2)
        {
            soll_temp_maischbottich2 = temp;
        }
        turn_on_heating(number);

        CallbackDataN* newdata = new CallbackDataN{number};
        einmaischtempertatur_erreicht_cb(nullptr, newdata); });

    heating_thread.detach();
}
