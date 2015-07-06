#include "param.hh"
void WithParameter::panel(int, Widget, Widget, char*) {}

#include "pedestrian.hh"
void Walker::draw(class SzeneFenster*, Drawable) {}
void WalkerPop::draw(class SzeneFenster*, Drawable) {}

#include "ground.hh"
void Ground::draw(class SzeneFenster *f, Drawable drw) {}

#include "street.hh"
void Wall::draw(class SzeneFenster *f, Drawable drw) {}
void CWall::draw(class SzeneFenster *f, Drawable drw) {}
void Decoration::draw(class SzeneFenster *f, Drawable drw) {}
void Street::draw(class SzeneFenster *f, Drawable drw) {}
void Gate::draw(class SzeneFenster *f, Drawable drw) {}

//Fenster
#include "simul.hh"
SimulObject *newSzeneFenster() {return NULL;} 
SimulObject *newVideoFenster() {return NULL;} 
SimulObject *newReplayFenster() {return NULL;} 

