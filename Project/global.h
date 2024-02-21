// GLOBAL_H

#ifndef GLOBAL_H
#define GLOBAL_H

#include <FL/Fl_Multiline_Output.H>


extern Fl_Output *istTemp;
extern Fl_Output *sollTemp;
extern Fl_Output *istTemp2;
extern Fl_Output *sollTemp2;

struct CallbackArgs {
    size_t index;
    int number;
};

struct CallbackDataBN {
    Fl_Window* window;
    int number;
    CallbackDataBN(Fl_Window* window, int number) : window(window), number(number) {}
};

struct CallbackDataBBN {
    Fl_Browser* browser;
    int number;
};

struct CallbackDataN {
    int number;
};

struct TempUpdateData {
    double temperature;
    Fl_Output* outputWidget;
};

#endif // GLOBAL_H