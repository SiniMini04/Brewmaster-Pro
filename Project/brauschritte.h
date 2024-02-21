// BRAUSCHRITTE_H

#ifndef BRAUSCHRITTE_H
#define BRAUSCHRITTE_H

#include <FL/Fl.H>
#include <fstream> 

void einmaischtempertatur_erreicht_cb(Fl_Widget *, void *v);
void rasten_einstellen_cb(Fl_Widget *, void *v);
void rast1_fertig_cb(Fl_Widget *, void *v);
void rast2_fertig_cb(Fl_Widget *, void *v);
void rast3_fertig_cb(Fl_Widget *, void *v);
void ablautertemperatur_einstellen_cb(Fl_Widget *, void *v);
void umpumpen_lauterbottich(Fl_Widget *, void *v);
void umpumpen_maischbottich_nach_lautern(Fl_Widget *, void *v);
void vorwurze_messen(Fl_Widget *, void *v);
void anschwanzen(Fl_Widget *, void *v);
void vollwurze_messen(Fl_Widget *, void *v);
void wuerzekochen_mit_hopfengabe(Fl_Widget *, void *v);
void wuerzekochen_mit_hopfengabe_2(Fl_Widget *, void *v);
void wirlen(Fl_Widget *, void *v);
void stammwurze_messen(Fl_Widget *, void *v);
void heizstangen_bedekt(Fl_Widget *, void *v);
void temp_zeit_einstellen_wuerzekochen(Fl_Widget *, void *v);
void timer_wirlen(Fl_Widget *, void *v);
void ausschlagen(Fl_Widget *, void *v);


#endif // BRAUSCHRITTE_H