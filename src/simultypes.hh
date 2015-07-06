#ifndef _SIMULTYPES_HH_
#define _SIMULTYPES_HH_

// ACHTUNG
// Es duerfen nur am Ende einer Teilkette neue Typen eingefuehrt werden!
// eine Aenderung der vorhandenen Typen zerstoert die abgespeicherten Movies
enum SimulObjectType { _UNDEF = 0, 
		       _DUMMY,
		       _SIMUL, _ACTOR,
		       _ENVIRONMENT = 100, _STREET, _CORRIDOR, _POLYGON, 
		       _CIRCLE, _OBSTACLE, 
		       _ATTRACTION = 200, 
		       _GROUND = 300, _MARKER,
		       _VERTEX = 400, _GATE, _LOCATION, _ENTRY, _DESTINATION, 
		       _EDGE, _MULTIPLEGATE, _DOOR, 
		       _WALKER = 500, _PEDESTRIAN, _WALKER_POP, _PED_POP, 
		       _DATAPEDEST, _EVALPEDEST, 
		       _FLOOR = 600, 
		       _NETWORK = 700,
		       _WINDOW = 1010, _SCENERY_WINDOW, _REPLAY_WINDOW, 
		       _NET_WINDOW, _HISTORY_WINDOW, _VIDEO_WINDOW,
		       _PLOT = 2010, _SCENERY_PLOT, _NET_PLOT, _CONTUR_PLOT,
		       _DECORATION = 4000,
		       _N_OBJECTS, _MOVIE, 
		       _MAX = 4095
		       };

#endif
