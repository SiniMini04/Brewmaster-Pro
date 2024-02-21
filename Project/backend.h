// BACKEND_H

#ifndef BACKEND_H
#define BACKEND_H

void turn_on_heating(int number);
void turn_off_heating(int number);
void turn_on_mixer(int number);
void turn_off_mixer(int number);
void mixer_button(int number);
void heating_button(int number);
nlohmann::json read_json_file(const std::string& folder ,const std::string& filename);
std::vector<std::string> get_recipe_files();
void change_einmaischtemperature_cb_backend(Fl_Window *window, int temp, int number);
void einmaischtempertatur_erreicht_cb_backend(Fl_Widget *w, void *v);
void rasten_einstellen_cb_backend(Fl_Window *window, int rast1_temp, int rast1_time, int rast2_temp, int rast2_time, int rast3_temp, int rast3_time, int number);
void rast2_backend(Fl_Widget* w, void* v);
void rast3_backend(Fl_Widget* w, void* v);
void ablautertemperatur_einstellen_cb_backend(Fl_Window *window, int temp, int number);
void lauterzeit_einstellen_cb_backend(Fl_Window *window, int time, int number);
void timer_vorwurze_backend(Fl_Window *window, int time, int number);
void vorwurze_messen_backend(Fl_Widget *w, void *v);
void temp_zeit_einstellen_wuerzekochen_backend(Fl_Widget *window, int time1,int time2, int temp, int number);
void wuerzekochen_mit_hopfengabe1_backend(Fl_Widget *window, void *v);
void wuerzekochen_mit_hopfengabe2_backend(Fl_Widget *window, void *v);
void timer_wirlen_backend(Fl_Window *window, int time, int number);
void timer_stammwuerze_messung_backend(Fl_Window *window, int time, int number);
void heizstangen_bedekt_backend(Fl_Window *window, int time, int number);
void close_heizstangen_bedekt_backend(Fl_Widget *w, void *v);
void close_umpumpen_in_maischbottich_fuer_wirlen(Fl_Widget *w, void *v);
void close_tank_angebeb(Fl_Widget *w, void *v);
void close_rast3(Fl_Widget *w, void *v);

#endif // BACKEND_H