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
#include <FL/Fl_Browser.H>
#include <FL/Fl_Check_Button.H>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <X11/Xlib.h>
#include <vector>
#include <string>
#include <dirent.h>
#include "global.h"
#include "backend.cpp"
#include "brauschritte.cpp"
#include "main.h"

Fl_Window *window;
Fl_Menu_Bar *menu;
Fl_Output *istTemp;
Fl_Output *istTemp2;
Fl_Output *sollTemp;
Fl_Output *sollTemp2;

/*

Frontend Functions

*/

// Callback function to close the application
void close_cb(Fl_Widget *, void *v)
{
    Fl_Window *window = (Fl_Window *)v;
    Fl::delete_widget(window);
}

// Callback function to open the about window
void about_cb(Fl_Widget *, void *)
{
    Fl_Window *about = new Fl_Window(340, 180);
    Fl_Box *box = new Fl_Box(20, 40, 300, 100, "Hello, World!\nThis is the About window.");
    box->labelsize(24);
    about->end();
    about->set_modal();
    about->show();
}

// Callback function to close the recipe window
void cancel_cb(Fl_Widget *, void *v)
{
    Fl_Window *recipe_window = (Fl_Window *)v;
    Fl::delete_widget(recipe_window);
}

void new_recipe_cb(Fl_Widget *, void *)
{

    int window_width = 600;
    int window_height = 440 + 30 + 20; // y-coordinate of the last element + height of the last element + some padding

    Fl_Window *recipe_window = new Fl_Window(window_width, window_height, "Neues Rezept");

    Fl_Box *name_label = new Fl_Box(20, 40, 100, 30, "Rezept Name:");
    Fl_Input *name_input = new Fl_Input(120, 40, 200, 30);

    Fl_Box *temp_label = new Fl_Box(20, 80, 200, 30, "Einmaisch Temperatur:");
    Fl_Int_Input *temp_input = new Fl_Int_Input(220, 80, 100, 30);
    Fl_Box *temp_unit = new Fl_Box(320, 80, 30, 30, "°C");

    Fl_Box *rast1_label = new Fl_Box(20, 120, 100, 30, "Rast 1:");
    Fl_Int_Input *rast1_temp_input = new Fl_Int_Input(150, 120, 100, 30, "Temp:");
    Fl_Box *rast1_temp_unit = new Fl_Box(250, 120, 30, 30, "°C");
    Fl_Int_Input *rast1_time_input = new Fl_Int_Input(360, 120, 100, 30, "Time:");
    Fl_Box *rast1_time_unit = new Fl_Box(460, 120, 50, 30, "min.");

    Fl_Box *rast2_label = new Fl_Box(20, 160, 100, 30, "Rast 2:");
    Fl_Int_Input *rast2_temp_input = new Fl_Int_Input(150, 160, 100, 30, "Temp:");
    Fl_Box *rast2_temp_unit = new Fl_Box(250, 160, 30, 30, "°C");
    Fl_Int_Input *rast2_time_input = new Fl_Int_Input(360, 160, 100, 30, "Time:");
    Fl_Box *rast2_time_unit = new Fl_Box(460, 160, 50, 30, "min.");

    Fl_Box *rast3_label = new Fl_Box(20, 200, 100, 30, "Rast 3:");
    Fl_Int_Input *rast3_temp_input = new Fl_Int_Input(150, 200, 100, 30, "Temp:");
    Fl_Box *rast3_temp_unit = new Fl_Box(250, 200, 30, 30, "°C");
    Fl_Int_Input *rast3_time_input = new Fl_Int_Input(360, 200, 100, 30, "Time:");
    Fl_Box *rast3_time_unit = new Fl_Box(460, 200, 50, 30, "min.");

    int button_width = 100;
    int button_spacing = (window_width - 2 * button_width) / 3;

    Fl_Button *cancel_button = new Fl_Button(button_spacing, 440, button_width, 30, "Abbrechen");
    cancel_button->callback(cancel_cb, recipe_window);
    Fl_Button *save_button = new Fl_Button(2 * button_spacing + button_width, 440, button_width, 30, "Speichern");
    save_button->callback(save_cb, recipe_window);

    recipe_window->end();
    recipe_window->set_modal();
    recipe_window->show();
}

void load_menu()
{
    std::vector<std::string> recipe_files = get_recipe_files();

    std::vector<nlohmann::json> recipes;

    for (const auto &file : recipe_files)
    {
        std::ifstream in("recipes/" + file);
        nlohmann::json recipe;
        in >> recipe;
        recipes.push_back(recipe);
    }

    // Clear the existing menu items
    menu->clear();

    // Add the static menu items
    menu->add("File/Quit", FL_ALT + FL_F + '4', close_cb, (void *)window, FL_MENU_DIVIDER);
    menu->add("Rezept/Neu", FL_CTRL + 'n', new_recipe_cb, 0, FL_MENU_DIVIDER);
    menu->add("Rezept/Bearbeiten", FL_CTRL + 'b', 0, 0, FL_MENU_DIVIDER);
    menu->add("Rezept/Löschen", FL_CTRL + 'l', 0, 0, FL_MENU_DIVIDER);

    // Add each recipe to the "Starten" submenu
    for (size_t i = 0; i < recipes.size(); ++i)
    {
        const auto &recipe = recipes[i];
        std::string menu_path = "Anlage/Maischbottich 1/Starten/" + recipe["name"].get<std::string>();
        CallbackArgs *args = new CallbackArgs{i, 1};
        menu->add(menu_path.c_str(), 0, choose_brauschritt_cb, (void *)args);
    }

    menu->add("Anlage/Maischbottich 1/Weitermachen", FL_CTRL + '3', 0, 0, FL_MENU_DIVIDER);
    menu->add("Anlage/Maischbottich 1/Pausieren", FL_CTRL + '2', 0, 0, FL_MENU_DIVIDER);
    menu->add("Anlage/Maischbottich 1/Stoppen", FL_CTRL + '4', 0, 0, FL_MENU_DIVIDER);

    for (size_t i = 0; i < recipes.size(); ++i)
    {
        const auto &recipe = recipes[i];
        std::string menu_path = "Anlage/Maischbottich 2/Starten/" + recipe["name"].get<std::string>();
        CallbackArgs *args = new CallbackArgs{i, 2};
        menu->add(menu_path.c_str(), 0, choose_brauschritt_cb, (void *)args);
    }

    menu->add("Anlage/Maischbottich 2/Pausieren", FL_CTRL + '6', 0, 0, FL_MENU_DIVIDER);
    menu->add("Anlage/Maischbottich 2/Weitermachen", FL_CTRL + '7', 0, 0, FL_MENU_DIVIDER);
    menu->add("Anlage/Maischbottich 2/Stoppen", FL_CTRL + '8', 0, 0, FL_MENU_DIVIDER);
    menu->add("Help/About", FL_CTRL + 'a', about_cb, 0, FL_MENU_DIVIDER);

    // Redraw the menu to show the new items
    menu->redraw();
}

int main(int argc, char **argv)
{
    XInitThreads();

    window = new Fl_Window(0, 0, Fl::w(), Fl::h(), "Hello World App");
    window->fullscreen();

    menu = new Fl_Menu_Bar(0, 0, Fl::w(), 30); // Create a new menu bar
    load_menu();

    Fl_PNG_Image *breweryTankImg = new Fl_PNG_Image("./img/brewerytank.png"); // Load the image
    Fl_Box *img1 = new Fl_Box(500, 400, 200, 200);
    Fl_Box *img2 = new Fl_Box(1200, 400, 200, 200);
    img1->image(breweryTankImg); // Display the image
    img2->image(breweryTankImg); // Display the image

    // Add two labels with Soll: and Ist: and a variable for the temperature for each of the two labels
    Fl_Box *sollLabel = new Fl_Box(490, 580, 100, 30, "Soll:");
    sollTemp = new Fl_Output(600, 580, 80, 30);

    Fl_Box *istLabel = new Fl_Box(490, 620, 100, 30, "Ist:");
    istTemp = new Fl_Output(600, 620, 80, 30);

    // Add two buttons at the bottom of the tank
    // Load the mixer image
    Fl_PNG_Image *mixerImg = new Fl_PNG_Image("./img/propeller.png");
    // Create the mixer button and set the mixer image
    Fl_Button *mixerButton = new Fl_Button(635, 680, 30, 30);
    mixerButton->image(mixerImg);
    mixerButton->callback(mixer_button, (void *)1);

    // Load the heater image
    Fl_PNG_Image *heaterImg = new Fl_PNG_Image("./img/heizung.png");
    // Create the heater button and set the heater image
    Fl_Button *heaterButton = new Fl_Button(535, 680, 30, 30);
    heaterButton->image(heaterImg);
    heaterButton->callback(heating_button, (void *)1);

    // Load the mixer image for the second tank
    Fl_PNG_Image *mixerImg2 = new Fl_PNG_Image("./img/propeller.png");
    // Create the mixer button for the second tank and set the mixer image
    Fl_Button *mixerButton2 = new Fl_Button(1335, 680, 30, 30);
    mixerButton2->image(mixerImg2);
    mixerButton2->callback(mixer_button, (void *)2);

    // Load the heater image for the second tank
    Fl_PNG_Image *heaterImg2 = new Fl_PNG_Image("./img/heizung.png");
    // Create the heater button for the second tank and set the heater image
    Fl_Button *heaterButton2 = new Fl_Button(1235, 680, 30, 30);
    heaterButton2->image(heaterImg2);
    heaterButton2->callback(heating_button, (void *)2);

    // Add two labels with Soll: and Ist: and a variable for the temperature for each of the two labels for the second tank
    Fl_Box *sollLabel2 = new Fl_Box(1190, 580, 100, 30, "Soll:");
    sollTemp2 = new Fl_Output(1300, 580, 80, 30);

    Fl_Box *istLabel2 = new Fl_Box(1190, 620, 100, 30, "Ist:");
    istTemp2 = new Fl_Output(1300, 620, 80, 30);

    update_temperature1_async();
    update_temperature2_async();
    update_soll_temperature();
    update_soll_temperature2();

    window->end();
    window->show(argc, argv);

    std::cout << "Starting the main loop" << std::endl;
    Fl::run();

    return 0;
}
