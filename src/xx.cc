#define NTEST
#include "test.h"
#include <stdlib.h>
#include <iostream.h>
#include <sys/types.h>
#include <unistd.h>

#include "vector.hh"
#include "list.hh"
#include "simul.hh"
#include "szfenster.hh"
#define XWINDOWS
#include "gui.hh"
#include "floor.hh"
#include "street.hh"
#include "pedestrian.hh"
#include "ground.hh"
#include "movie.hh"

#ifdef UNIRAS
#include <agX/agxgra.h>
#include "plot.hh"
#endif

GUI *userinterface;

main (unsigned int argc, char **argv) {
  pid_t pid;
  char text[20];
  GUI ui(argc, argv);
  userinterface = &ui;
  SimulObjectNode anker(0, _SIMUL, "Pedestrian-Facility"); 
  root = (Simul*)anker.obj;
  SzeneFenster *myf;
  //  Display *display;
  XEvent event_return;
  Arg arglist[3];
  int n, NSnapShot;
  RemoteControl *rc;
  Floor *floor;
  Pointer<Floor> flo;
  Fenster *fenster;
#ifdef UNIRAS
  Plot *plot;
  List activeplots;
  Pointer<Plot> pl;
#endif
  
#ifdef UNIRAS
  // Initalize the agX Toolkit
  if (!ui.CF.TextMode) {
    XuInitialize(argc, argv);
  }
#endif
  
  if (!ui.CF.TextMode) {
    rc = new RemoteControl(&ui);
  }
  // main loop
  
  Pointer<Floor> pfloor;
  NSnapShot = 0;
  while (!ui.CF.Exit || ui.ToDo()) {
    if (!ui.CF.TextMode && XtAppPending(ui.context)) {
      XtAppNextEvent(ui.context, &event_return);
      XtDispatchEvent(&event_return); 
    }
    else if (ui.ToDo()) {
      switch (ui.NextDo()) {
      case FileOpenDialog:
	ui.CF.reset();
	Manage("FileOpenDialog");
	break;
	
      case ReadSimul:
	if (ui.Npedfiles) {
	  for (int i=0; i<ui.Npedfiles; ++i) {
	    ui.Pedfp = new TextFile(ui.PedFileName[i], "r");
	    if (ui.Pedfp->isopen()) {
	      anker.parse(ui.Pedfp);
	    }
	    delete ui.Pedfp;
	  }
	}
	// else {}
	flo = root->floors; floor = flo;
	//if (floor) {
	//Unmanage("FileOpenDialog");
	anker.obj->init();
	if (!ui.CF.TextMode && root->displays.N()==0) {
	  ui.AddDo(OpenSzeneFenster);
	}
	
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
	for (Pointer<Movie> mov=root->movies; mov; ++mov) {
	  mov->snap(immediate);
	}
	break;

      case OpenSzeneFenster: 
	myf = new SzeneFenster();
	root->ref(myf);
	myf->ref(floor);
	myf->init();
	test(printf("create fenster new=%d\n", myf)); 
	//	ui.fenster.append((ListElement*)myf);
	ui.fenster.append(myf);
	break;
	
      case OpenNetworkFenster:
	{
	 // NetFenster *f = new NetFenster(ui.display, floor->network);
	 // ui.fenster.append(f);
	}
	break;
	
      case UpdateDisplay:
	// myf->draw();
	if (!ui.CF.TextMode) {
	  //for (Pointer<Fenster> ff=ui.fenster; ff; ++ff) {
	  for (Pointer<Fenster> ff=root->displays; ff; ++ff) {
	    //test(printf("update fenster nr %d\n", ff->nr));
	    ff->draw();
	  }  
	}
	else {
	  printf("xu: %s T=%lf\n", ui.PedFileName, root->T);
	}
	break;
#ifdef _PLOT_HH_
      case DoTopViewPlot:
	for (pl = root->plots; pl; ++pl) {
	  if (pl->objecttype() == _SCENERY_PLOT) break;
	}
	if (pl) plot = pl;
	else {
	  announce("new scenery plot\n");
	  plot = new SceneryPlot();
	  root->plots.append(plot);
	  Pointer<Floor> flo = root->floors;
	  plot->ref(flo);
	  plot->init();
	}
	Manage("PlotTopViewPanel", plot);
	break;
	
      case DoGateStatPlot:
	if (plot) {
	  ui.Message("Please finish activated plot first.");
	  break;
	}
	break;
	
      case DoConturPlot:
	if (plot) {
	  ui.Message("Please finish activated plot first.");
	  break;
	}
	break;
	
      case DoPlot:
	announce("plot ...");
	plot->parameter->write(stdout);
	if (plot) {
	  plot->plot(); 
	  //if (!plot->done()) activeplots.append(plot); 
	}
      case CancelPlot:
	plot = NULL;
	announce("done.\n");
	break;
      case DoPlotAll:
	for (pl=root->plots; pl; ++pl) {
	  pl->plot(); 
	  //if(!pl->done()) activeplots.append(*pl);
	}
	break;
	/* case DoActivePlots:
	for (pl=root->plots; pl; ++pl) {
	if (pl->running()) pl->plot();
	}
	break; */
#endif
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
      //      if (((Simul*)anker.obj)->floors.N()) { // Simulation Time Step 
      if ( ui.CF.RunSimulation || ui.CF.StepSimulation ) {
	if (ui.CF.Replay) { // REPLAY 
	  for (Pointer<Movie> mov=root->movies; mov; ++mov) {
	    mov->forw();
	  }
	}
	else {  // calculation 
	  n = 0;
	  for (flo = ((Simul*)anker.obj)->floors; flo; ++flo) {
	    n += flo->update();
	  } // flo-loop
	} // calculation
	//if (ui.ControlerFlag & CFWriteData) floor->bwrite(ui.Replayfp);
	/* Real Simulation */
	root->T += root->Dt;
	char timetxt[7];
	sprintf(timetxt, "%06.0lf", root->T*10.0);
	setenv("XU_TIME", timetxt, 1);
	++root->counter;
	if (ui.CF.StepSimulation ||
	    (root->display_rate>0  && root->T >= root->display_wait
	     && root->counter % root->display_rate==0)) {
	  ui.AddDo(UpdateDisplay);
	}

	// ui.AddDo(DoActivePlots); // wird immer durchgefuehrt 

	if (root->plot_rate>0 && root->T >= root->plot_wait
	    && root->counter % root->plot_rate==0) {
	  ui.AddDo(DoPlotAll);
	}

	for (Pointer<Movie> mov=root->movies; mov; ++mov) {
	  mov->snap(TimeStep);
	}

	if (root->log_rate>0 && root->counter % root->log_rate==0) {
	  for (flo = ((Simul*)anker.obj)->floors; flo; ++flo) flo->log();
	}

	NSnapShot++;
	if (!n) ui.CF.RunSimulation = 0;
	ui.CF.StepSimulation = 0;
	if (root->Tstop>0.0 && root->T>root->Tstop) {
	  if (ui.CF.TextMode) ui.CF.Exit = 1;
	  else ui.CF.RunSimulation = 0;
	}
      }
      //  } /* end of if floor */
      /* Platz fuer weitere Aktivitaeten */    
    } // ACTION-LOOP
  } // end of while-ControlerFlag-loop 
  
  /* Aufraeumen! */
  fclose(ui.Replayfp);
  delete floor;
  /* for (f=fenster; f; f=f->succ) {
     XCloseDisplay(f->dis);
     } */
  /* FreeBuilding(floor, customize);
     FreeBitmap(floor, customize); */
}















