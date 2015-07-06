#ifndef _USERINTERF_HH_
#define _USERINTERF_HH_

#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>

#include "list.hh"
#include "param.hh"

struct ControlerFlag {
  unsigned int RunSimulation   : 1;
  unsigned int WaitForBuilding : 1;
  unsigned int StepSimulation  : 1;
  unsigned int Snapshot        : 1;      
  unsigned int Replay          : 1;         
  unsigned int StepReplay      : 1;   
  unsigned int ForwReplay      : 1;   
  unsigned int RewReplay       : 1;  
  unsigned int Title           : 1;         
  unsigned int Exit            : 1;      
  unsigned int TextMode        : 1; 
  unsigned int WriteData       : 1;
  unsigned int : sizeof(int)*8-12;
  void reset() {
    RunSimulation   = 0;
    WaitForBuilding = 0;
    StepSimulation  = 0;
    Snapshot        = 0;    
    Replay          = 0;    
    StepReplay      = 0; 
    ForwReplay      = 0;
    RewReplay       = 0;
    Title           = 0;
    Exit            = 0;
    TextMode        = 0;
    WriteData       = 0;
  }
};


// DO 
enum DO {Nop, FileOpenDialog, FileSaveDialog, ParameterEditor,
	 ReadSimul, ResetSimul, CalculateSimul, WriteSimul,
	 OpenSzeneFenster, OpenHistoryFenster, OpenStatFenster,
	 OpenNetworkFenster, UpdateDisplay, CloseAllFenster,
	 DoTopViewPlot, DoGateStatPlot, DoConturPlot, DoPlot, CancelPlot,
	 DoPlotAll, DoActivePlots, OpenLogfile, CloseLogfile,
	 Snap, MovieFileOpen, MovieFileClose, Replay};
	 

#define DO_LIST_MAX 100

class TextFile; 

class UI {
private:
  DO DoList[DO_LIST_MAX];
  int NextDoIndex, AddDoIndex;

public:
  ControlerFlag CF;
  char *PedFileName, *ReplayFileName, *LogFileName;
  //FILE
  TextFile *Pedfp;
  FILE *Replayfp, *Logfp;

  virtual void Message (char *txt) {
    cout << txt << '\n';
  }
  virtual void Error (char *txt) {
    cout << "ERROR: " << txt << '\n';
    exit(1);
  }
  virtual void Warning (char *txt) {
    cout << "Warning: " << txt << '\n';
  }
  void ClearDo() { NextDoIndex = AddDoIndex = 0; }
  DO NextDo() {
    NextDoIndex = (NextDoIndex+1) % DO_LIST_MAX;
    return DoList[NextDoIndex];
  }
  void AddDo(DO task) {
    AddDoIndex = (AddDoIndex+1) % DO_LIST_MAX;
    DoList[AddDoIndex] = task;
  }
  void UniqDo(DO);
  bool ToDo() {return NextDoIndex != AddDoIndex;}
  virtual ~UI();
};

#endif // _GUI_HH_






