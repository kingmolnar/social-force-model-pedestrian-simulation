#ifndef _NETPLOT_HH_
#define _NETPLOT_HH_

#include "param.hh"

class Network;

class NetPlot: public WithParameter {
public:
  NetPlot(Parameter*, Network*);
  ~NetPlot();
};
#endif

