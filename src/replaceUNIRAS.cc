#include "pedestrian.hh"
void Walker::hardcopy(class SceneryPlot*) {}
void Walker::trace(class SceneryPlot*) {}
void WalkerPop::hardcopy(class SceneryPlot*) {}
void WalkerPop::trace(class SceneryPlot*) {}

#include "ground.hh"
void Ground::hardcopy(class SceneryPlot*) {}

#include "street.hh"
void Wall::hardcopy(class SceneryPlot*) {}
void CWall::hardcopy(class SceneryPlot*) {}
void Decoration::hardcopy(class SceneryPlot*) {}
void Street::hardcopy(class SceneryPlot*) {}
void Gate::hardcopy(class SceneryPlot*) {}

//Plot
#include "simul.hh"
SimulObject *newSceneryPlot() {} 
SimulObject *newConturPlot() {} 

