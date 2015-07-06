#define NTEST
#include "test.h"
#include <stdlib.h>
#include <iostream.h>
#include <sys/types.h>
#include <unistd.h>

#include "vector.hh"
#include "list.hh"
#include "simul.hh"
#include "gui.hh"
#include "floor.hh"
#include "street.hh"
#include "pedestrian.hh"
#include "ground.hh"
#include "movie.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */

UI *userinterface;

//main (unsigned int argc, char **argv) {
int main (unsigned int argc, char **argv) {
  pid_t pid;
  char text[20];
  UI ui(argc, argv);
  userinterface = &ui;
  SimulObjectNode anker(0, _SIMUL, "Pedestrian-Facility"); 
  // Module einhaengen
  SimulCreator* sc;
  sc = new SimulCreatorFloor; anker.creators.append(sc);
  sc = new SimulCreatorPedestrian; anker.creators.append(sc);
  sc = new SimulCreatorStreet; anker.creators.append(sc);
  sc = new SimulCreatorGround; anker.creators.append(sc);
  root = (Simul*)anker.obj;
  Floor *floor;
  Pointer<Floor> flo;
  Pointer<Movie> mov;
  int n;
  
  // main loop

  while (!ui.CF.Exit || ui.ToDo()) {
    if (0 /* ui.SignalPending() */ ) {
    }
    else if (ui.ToDo()) {
      switch (ui.NextDo()) {
      case FileOpenDialog:
	break;
	
      case ReadSimul:
	ui.Message("read:\n");
	if (ui.Npedfiles) {
	  for (int i=0; i<ui.Npedfiles; ++i) {
	    ui.Pedfp = new TextFile(ui.PedFileName[i], "r");
	    if (ui.Pedfp->isopen()) {
	      anker.parse(ui.Pedfp);
	    }
	    delete ui.Pedfp;
	  }
	  flo = root->floors; floor = flo;
	  anker.obj->init();
	  ui.CF.RunSimulation = 1;
	}
	else {
	  ui.CF.Exit = 1;
	  ui.Message("Sorry!\n");
	}
       	
#ifndef NTEST
	// Ausgabe ueberppruefen
	for (flo = root->floors; flo; ++flo) {
	  for (Pointer<Street> str=flo->streets; str; ++str) {
	    for (Pointer<Obstacle> obs=str->obstacles; obs; ++obs) {
	      obs->print(stdout);
	    }
	  }
	  for (Pointer<Gate> gat=flo->gates; gat; ++gat) {
	    gat->print(stdout);
	  }
	}
#endif
	break;
	
      case ResetSimul:
	if (root) {
	  for (Pointer<Walker> ac=root->walkers; ac; ++ac) {
	    ac->init();
	  }
	  root->init();
	}
	ui.AddDo(UpdateDisplay);
	break;
	
      case CalculateSimul:
	for (flo = root->floors; flo; ++flo){
	  pid = 0;
	  /* pid = fork(); */
	  if (pid==0) {
	    //	      (*floor->calc)(floor);
	    /* ControlerFlag = CFExit; */
	  }
	}
	break;
	
      case WriteSimul:
	break;
	
      case Snap:
	for (mov=root->movies; mov; ++mov) {
	  mov->snap(immediate);
 	}
	break;
	
	
      case UpdateDisplay:
	{
	  char s[80];
	  sprintf(s, "xu: %s T=%lf\n\n", ui.PedFileName[0], root->T);
	  ui.Message(s);
	}
	break;
	
      case OpenLogfile:
	ui.Logfp = fopen("LOGFILE", "a");
	break;

      case CloseLogfile:
	fclose(ui.Logfp);
	ui.Logfp = NULL;
	break;
      }
    } // end todo 
    
    else { // ACTION-LOOP
      if (ui.CF.RunSimulation) {
	n = 0;
	for (flo = ((Simul*)anker.obj)->floors; flo; ++flo) {
	  n += flo->update();
	} // flo-loop
	for (mov=root->movies; mov; ++mov) {
	  mov->snap(TimeStep);
	} // mov-loop
      } // calculation
      root->T += root->Dt;
      //char timetxt[7];
      //sprintf(timetxt, "%06.0lf", root->T*10.0);
      //setenv("XU_TIME", timetxt, 1);
      char timetxt[80];
      //sprintf(timetxt, "XU_TIME=%06.0lf", root->T*10.0);
      putenv(timetxt);
      
      ++root->counter;
      if (ui.CF.StepSimulation ||
	  (root->display_rate>0  && root->T >= root->display_wait
	   && root->counter % root->display_rate==0)) {
	ui.AddDo(UpdateDisplay);
      }
      
//     	Record Rates are handled by the movie objects themselves.
//	The appropriate routines are called in the action loop
//      if (root->record_rate>0  && root->T >= root->record_wait
//	  && root->counter % root->record_rate==0) {
//	ui.AddDo(Snap);
//      } 

      if (root->log_rate>0 && root->counter % root->log_rate==0) {
	for (flo = ((Simul*)anker.obj)->floors; flo; ++flo) flo->log();
      }
      
      if (!n) ui.CF.RunSimulation = 0;
      if (root->Tstop>0.0 && root->T>root->Tstop) {
	for (flo = ((Simul*)anker.obj)->floors; flo; ++flo) flo->log();
	ui.CF.Exit = 1;
      }
      /* Platz fuer weitere Aktivitaeten */    
    } // ACTION-LOOP
  } // end of while-ControlerFlag-loop 
  
  /* Aufraeumen! */
  fclose(ui.Replayfp);
  delete floor;
}












