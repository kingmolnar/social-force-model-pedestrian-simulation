#ifndef _GUI_HH_
#define _GUI_HH_

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
	 Snap, MovieFileOpen, MovieFileClose, Replay,
	 ViewBack, ViewGoto, ViewReset};
	 

#define DO_LIST_MAX 100

class TextFile; 

class UI {
private:
  DO DoList[DO_LIST_MAX];
  int NextDoIndex, AddDoIndex;

public:
  ControlerFlag CF;
  char *PedFileName[10], *ReplayFileName, *LogFileName;
  int Npedfiles;
  //FILE
  TextFile *Pedfp;
  FILE *Replayfp, *Logfp;

  // methods
  UI(int = 0, char** = 0);
  ~UI();
  virtual void commandline(int, char**);
  virtual void Message (char *txt) {
    cout << txt << '\n' << flush;;
  }
  virtual void Error (char *txt) {
    cerr << "ERROR: " << txt << '\n' << flush;
    exit(1);
  }
  virtual void Warning (char *txt) {
    cout << "Warning: " << txt << '\n' << flush;
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
};

#endif // _USERINTERF_HH_

#ifdef XWINDOWS

extern "C" {
#define new new_
#define class class_
#include <Mrm/MrmPublic.h>
#include <X11/Xt/Intrinsic.h>
#include <Xm/Xm.h>
#include <X11/Xlib.h>
#include <X11/X.h>

#undef new
#undef class
}

// Hash-Table
int HashFunction(char*, int);
int HashLookup(char*, Widget*);
int HashRegister(char*, Widget);
void Manage(char*);
//void Manage(char*, XtArgVal);
void Manage(char*, WithParameter*);
void Manage(char*, Arg*, int);
void Unmanage(char*);
XtArgVal UserData(Widget);
XtArgVal UserData(char *);

class GUI: public UI {
friend int HashFunction(char*, int);
friend int HashLookup(char*, Widget*);
friend int HashRegister(char*, Widget);
friend void Manage(char*);
friend void Manage(char*, WithParameter*);
friend void Manage(char*, Arg*, int);
friend void Unmanage(char*);

private:
  MrmHierarchy hierarchy;	             // MRM database hierarchy ID   
public:
  // Motif Global variables 
  Display         *display;	 // Display variable 
  XtAppContext    context;	 // application context 
  Widget	  toplevel;      // Root widget ID of application 
  List fenster;
  void MessageLine(char*);
  GUI(int, char**);
  ~GUI();
};

class RemoteControl: public WithParameter {
public:
  GUI *ui;
  RemoteControl(GUI*);
  void init() {}; // vorerst nichts zu tun
  //  void panel (int task, Widget w, Widget par, char *tag);
  void panelActivate(Widget w, Widget par, char *tag);
  void message(char*);
};

class ViewerControl: public WithParameter {
public:
  GUI *ui;
  ViewerControl(GUI*);
  void init() {}; // vorerst nichts zu tun
  //  void panel (int task, Widget w, Widget par, char *tag);
  void panelActivate(Widget w, Widget par, char *tag);
  void message(char*);
};

class FileDialog: public WithParameter {
public:
  GUI *ui;
  FileDialog(GUI*);
  void panel (int task, Widget w, Widget par, char *tag);
};

#endif // XWINDOWS

#endif // _GUI_HH_






