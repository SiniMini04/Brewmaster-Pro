// MAIN_H

#ifndef MAIN_H
#define MAIN_H

#include <FL/Fl_Menu_Bar.H>
#include <vector>
#include <nlohmann/json.hpp>

void load_menu();
void close_cb(Fl_Widget *, void *);
void cancel_cb(Fl_Widget *, void *);

#endif // MAIN_H