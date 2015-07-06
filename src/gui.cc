#define NTEST
#include "test.h"
#include "gui.hh"
#include "hash.hh"


extern "C" {
  int strcasecmp(char*, char*);
  //  char *strncpy(char*, char*, unsigned int);
  char *index(char *s, char c);
  char *rindex(char *s, char c);
}

// UserInterface
UI::UI(int argc, char **argv) {
  ClearDo();
  if (argc) {
    commandline(argc, argv);
    if (Npedfiles==0) {
      Error("file open error.\n");
    }
    else {
      AddDo(ReadSimul);
    }
  }
}
  

UI::~UI()
{
}

void UI::commandline(int argc, char **argv) {
  Npedfiles = 0;
  for (int n=1; n<argc; n++) {
    if (*argv[n]=='-') {
      switch(*(argv[n]+1)) {
      case 'r':
	CF.RunSimulation = 1;
	break;
      case 'T':
	CF.TextMode = 1;
	break;
      case 'f': /* Replay */
	n++;
	Replayfp = NULL;
	if (n<argc) {
	  ReplayFileName = argv[n];
	  if (strcmp(ReplayFileName, "-")==0) Replayfp = stdin;
	  else Replayfp = fopen(ReplayFileName, "r");
	  //sprintf(mess, "Open replay file %s\n", ReplayFileName);
	}
	if (Replayfp) {	
	  //MessageLine(mess);
	  CF.Replay = 1;
	  CF.WriteData = 0; 
	}
	break;
      case 'o': /* Write */
	n++;
	Replayfp = NULL;
	if (n<argc) {
	  ReplayFileName = argv[n];
	  if (strcmp(ReplayFileName,"-")==0) Replayfp = stdout;
	  else Replayfp = fopen(ReplayFileName, "w");
	}
	if (Replayfp) {	  
	  CF.WriteData = 1;
	  CF.Replay = 0;
	}
	break;
      case 's': /* LOGFILE */
	AddDo(OpenLogfile);
      default:
	break;
      }
    }
    else {
      PedFileName[Npedfiles++] = argv[n];
    }
  }
  //  cout << Npedfiles << " script file(s) read." << endl;
}


void UI::UniqDo(DO task) {
  int a = NextDoIndex;
  int z = AddDoIndex;
  if (z<a) z += DO_LIST_MAX;
  while (a<=z) {
    if (DoList[a%DO_LIST_MAX]==task) return;
    a++;
  }
  AddDoIndex = (AddDoIndex+1) % DO_LIST_MAX;
  DoList[AddDoIndex] = task;
}
  
#ifdef XWINDOWS

#include "plot.hh"

extern "C" {
#define new new_
#define class class_
#include <Xt/Intrinsic.h>
#include <Xm/Xm.h>
#include <Mrm/MrmAppl.h>            /* Motif Toolkit and MRM */
#undef new
#undef class
}

// global defs
bool isMrmInitialized = 0;

#define hash_table_limit 500
static struct HASH_TABLE_STRUCT {
  char	*widget_name;
  Widget	id;
} hash_table[hash_table_limit + 1];
static MrmType class_id;		     // Place to keep class ID

#define withparameter_table_limit 100
static struct WITHPARAMETER_STRUCT {
  Widget id;
  WithParameter *p;
} withparameter_table[withparameter_table_limit+1];
static int withparameter_table_n;

struct ParamWid {
  Widget w;
  WithParameter *p;
};

Hash<ParamWid> ParamWidTable;

static WithParameter* TableLookup(Widget w) {
  for (int i=0; i<withparameter_table_n; ++i) {
    if (withparameter_table[i].id==w)
      return withparameter_table[i].p;
  }
  return 0;
}

static int TableRegister(Widget w, WithParameter *p) {
  WithParameter *t;
  for (int i=0; i<withparameter_table_n; ++i) {
    if (withparameter_table[i].id==w) {
      withparameter_table[i].p = p;
      return 1;
    }
  }
  if (withparameter_table_n<=withparameter_table_limit) {
    withparameter_table[withparameter_table_n].id = w;
    withparameter_table[withparameter_table_n].p = p;
    ++withparameter_table_n;
    return 1;
  }
  else return 0;
}

static struct {
  Display *display;	 // Display variable 
  XtAppContext context;	 // application context 
  Widget toplevel;      // Root widget ID of application 
  MrmHierarchy hierarchy;	             // MRM database hierarchy ID 
  GUI *ui;
} g;

// utilities
void GetSomething(Widget, char*, void*);
void SetSomething(Widget, char*, void*);
int CSText(char**,  XmString);

// Callback Procedures 
void NYI(Widget, int*, unsigned long*);
void UnmanageProc(Widget, int*, unsigned long*);

void CreateProc (Widget w, char *tag, unsigned long reason) {
  Widget wid;
  WithParameter *wp;
  for (wid=w; wid; wid = XtParent(wid)) {
    wp = TableLookup(wid);
    if (wp) {
      wp->panelCreate(w, wid, tag);
      return;
    }
  }
  printf("Sorry, no appropriate widget found.\n");
}

void ValueProc (Widget w, char *tag, unsigned long reason) {
  Widget wid;
  WithParameter *wp;
  for (wid=w; wid!=0; wid = XtParent(wid)) {
    wp = TableLookup(wid);
    if (wp) {
      wp->panelValue(w, wid, tag);
      return;
    }
  }
  printf("Sorry, no appropriate widget found.\n");
}

void ActivateProc (Widget w, char *tag, unsigned long reason) {
  Widget wid;
  WithParameter *wp;
  for (wid=w; wid!=0; wid = XtParent(wid)) {
    wp=TableLookup(wid);
    if (wp) {
      wp->panelActivate(w, wid, tag); //3
      return;
    }
  }
  printf("Sorry, no appropriate widget found.\n");
}

void RegisterProc (Widget w, char *tag, unsigned long reason) {
  ParamWid *a;
  if (ParamWidTable.lookup(tag, &a) && a && a->p) {
    TableRegister(w, a->p);
    test(printf("Register Widget `%s\'\n", tag));
  }
  else printf("Sorry, can't register widget `%s\'.\n", tag);
}

                             
GUI::GUI(int argc, char **argv)
{ 
  /* local defs */
  announce("construct GUI...");
  g.ui = this;

  ClearDo();
  
  commandline(argc, argv);
  if (PedFileName==0) {
    AddDo(FileOpenDialog);
  }
  else {
    AddDo(ReadSimul);
  }
  
  /* Start */
  if (!CF.TextMode) {
    char *db_filename_vec[] = {"xx.uid"};
    int db_filename_num =
      (sizeof db_filename_vec / sizeof db_filename_vec [0]);
    MrmInitialize();			
    XtToolkitInitialize();
    g.context = context = XtCreateApplicationContext();
    g.display = display = XtOpenDisplay(context, NULL, argv[0], "example",
					0, 0, &argc, argv);
    if (g.display == 0) {
      fprintf(stderr, "%s:  Can't open display\n", argv[0]);
      exit(1);
    }
    
    Arg arglist[3];
    int n = 0;
    XtSetArg(arglist[n], XmNallowShellResize, True);  n++;
    XtSetArg(arglist[n], XmNheight, 10); n++;
    XtSetArg(arglist[n], XmNwidth, 10); n++;
    g.toplevel = toplevel = XtAppCreateShell(argv[0], NULL, 
					     applicationShellWidgetClass,
					     g.display, arglist, n);
    
    /* 
     * Open the UID files (the output of the UIL compiler) in the hierarchy
     */
    int status = 
      MrmOpenHierarchy(db_filename_num, db_filename_vec, 0, &hierarchy);
    if (status!= MrmSUCCESS) {
      printf("can't open hierarchy\n");
    }
    
    /* Names and addresses of callback routines to register with Mrm */
    MrmRegisterArg reglist [] = { { "UnmanageProc", (caddr_t)&UnmanageProc},
				  { "RegisterProc", (caddr_t)&RegisterProc},
				  { "ActivateProc", (caddr_t)&ActivateProc},
				  { "CreateProc", (caddr_t)&CreateProc},
				  { "ValueProc", (caddr_t)&ValueProc},
				  { "NYI", (caddr_t)&NYI} 
				};
    int reglist_num = (sizeof reglist / sizeof(MrmRegisterArg));
    status = MrmRegisterNames (reglist, reglist_num);
    if (status!= MrmSUCCESS) {
      printf("can't register functions\n");
    }
    
    g.hierarchy = hierarchy;
    XtRealizeWidget(g.toplevel);
  } // X-stuff
  test(cout << "done\n");
} // end of init


GUI::~GUI() 
{
}


void Manage(char* widget_name) {
  Widget		id;
  Window		pop_window;
  XWindowChanges	values;
  
  if (HashLookup(widget_name, &id))
    if (XtIsManaged(id)) {
      pop_window = XtWindow(XtParent(id));
      values.x = values.y = values.width = values.height =
	values.border_width = values.sibling = NULL;
      values.stack_mode = Above;
      XConfigureWindow(g.display, pop_window, CWStackMode, &values);
    }
    else
      XtManageChild(id);
  else {
    MrmFetchWidget(g.hierarchy, widget_name, g.toplevel,
		   &id, &class_id);
    XtManageChild(id);
    HashRegister(widget_name, id);
  }
}

void Manage(char* widget_name, WithParameter *wp) {
  Widget		id;
  Window		pop_window;
  XWindowChanges	values;
  ParamWid a;
  a.p = wp; a.w = 0;
  ParamWidTable.regist(widget_name, &a);
  if (HashLookup(widget_name, &id)) {
    //TableRegister(id, wp);
    if (XtIsManaged(id)) {
      pop_window = XtWindow(XtParent(id));
      values.x = values.y = values.width = values.height =
	values.border_width = values.sibling = NULL;
      values.stack_mode = Above;
      XConfigureWindow(g.display, pop_window, CWStackMode, &values);
    }
    else XtManageChild(id);
  }
  else {
    MrmFetchWidget(g.hierarchy, widget_name, g.toplevel, 
		   &id, &class_id);
    XtManageChild(id);
    HashRegister(widget_name, id);
    //TableRegister(id, wp);
  }
}


void Manage(char *widget_name, Arg *args, int n) {
  Widget		id;
  Window		pop_window;
  XWindowChanges	values;
  
  if (HashLookup(widget_name, &id)) {
    if (XtIsManaged(id)) {
      pop_window = XtWindow(XtParent(id));
      values.x = values.y = values.width = values.height =
	values.border_width = values.sibling = NULL;
      values.stack_mode = Above;
      XConfigureWindow(g.display, pop_window, CWStackMode, &values);
    }
    else
      XtManageChild(id);
  }
  else {
    MrmFetchWidget(g.hierarchy, widget_name, g.toplevel, &id, 
		   &class_id); 
    XtSetValues(id, args, n);
    XtManageChild(id);
    HashRegister(widget_name, id);
  }
  XtSetValues(id, args, n);
}

void Unmanage(char *widget_name)
{
  Widget id;
  if (HashLookup(widget_name, &id)) {
    XtUnmanageChild(id);
    //XtDestroyWidget(id);
  }
}

int HashRegister (char *widget_name, Widget id)    
{
  int ndx;
  for (ndx = HashFunction(widget_name, hash_table_limit);
       ((hash_table[ndx].widget_name != NULL) &&
	(ndx < hash_table_limit));
       ndx++);
  if (hash_table[ndx].widget_name != NULL)
    for (ndx = 0;
	 ((hash_table[ndx].widget_name != NULL) &&
	  (ndx < hash_table_limit));
	 ndx++);
  if (ndx == hash_table_limit)
    return 0;
  else {
    hash_table[ndx].widget_name = XtCalloc(1, strlen(widget_name) + 1);
    strcpy(hash_table[ndx].widget_name, widget_name);
    hash_table[ndx].id = id;
    return 1;
  }
}


int HashLookup (char *name, Widget *id) {
  int ndx;
  for (ndx = HashFunction(name, hash_table_limit);
       ((hash_table[ndx].widget_name != NULL) && (ndx <= hash_table_limit));
       ndx++)
    if (strcmp(name, hash_table[ndx].widget_name) == 0) {
      *id = hash_table[ndx].id;
      return 1;
    }

  if (ndx > hash_table_limit)
    for (ndx = 0;
	 ((hash_table[ndx].widget_name != NULL) && (ndx <= hash_table_limit));
	 ndx++)
      {
	if (strcmp(name, hash_table[ndx].widget_name) == 0) {
	  *id = hash_table[ndx].id;
	  return 1;
	}
      }
  
  return 0;
}

int HashFunction (
    char		*name,
    int			max)

{
#define HashVecSize		20	/* plenty for 31 character names */
  typedef union {
    short int		intname[HashVecSize];	 /* name as vector of ints */
    char		charname[2*HashVecSize]; /* name as vector of chars */
  } HashName;

  HashName		locname;	/* aligned name */
  int			namelen;	/* length of name */
  int			namelim;	/* length limit (fullword size) */
  int			namextra;	/* limit factor remainder */
  int			code = 0;	/* hash code value */
  int			ndx;		/* loop index */
  
  /*
   * Copy the name into the local aligned union.
   * Process the name as a vector of integers, with some remaining characters.
   * The string is copied into a local union in order to force correct
   * alignment for alignment-sensitive processors.
   */
  strcpy (locname.charname, name);
  namelen = strlen (locname.charname);
  namelim = namelen >> 1;		/* divide by 2 */
  namextra = namelen & 1;		/* remainder */
  
  /*
   * XOR each integer part of the name together, followed by the trailing
   * 0/1 character
   */
  for ( ndx=0 ; ndx<namelim ; ndx++ )
    code = code ^ ((locname.intname[ndx])<<ndx);
  if ( namextra > 0 )
    code = code ^ ((locname.intname[ndx])&0x00FF);
  
  return (code&0x7FFF) % max;
}


void GetSomething (Widget w, char *resource, void* value)
{
  Arg al[1];
  XtSetArg(al[0], resource, (XtArgVal*)value);
  XtGetValues(w, al, 1);
}

void SetSomething (Widget w, char *resource, void* value)
{
  Arg al[1];
  XtSetArg(al[0], resource, (XtArgVal*) value);
  XtSetValues(w, al, 1);
}

int CSText(char **ascii, XmString comp)
{
  XmStringContext context;
  XmStringCharSet chars;
  XmStringDirection direc;
  Boolean seper, status;
  
  status = XmStringInitContext(&context, comp);
  if (!status) {
    //Message("string convertion error in CSText()");
  }
  else {
    XmStringGetNextSegment(context, ascii, &chars, &direc, &seper);
  }
  XmStringFreeContext(context);
  return(0);
}

/* void GUI::Message(char *text)
{
  printf("%s\n", text);
}
*/

/* USER DEFINED Callbacks */

void FileOpenProc (
Widget		w,
int		*tag,
unsigned long	*reason)
{
}
 
FileDialog::FileDialog(GUI *interface) {
  ParameterDef d[] = { {"filename", ""},
		       {"action", "1"}
		     };
  parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  ui = interface;
}

void FileDialog::panel(int task, Widget w, Widget par, char *tag) {
  Arg args[20];
  int i, n;
  XmString filename;
  char *fname;
  FILE *fp;
  Widget rc;
  struct Street *s;
//  struct Plot *pl;
  switch (task) {
  case 1: // create
    break;
  case 2: // value
    if (strcasecmp(tag, "filename")==0) {
      GetSomething(w, XmNdirSpec, &filename);
      CSText(&fname, filename);
      parameter->set("filename", fname);
    }
    break;
  case 3: //activate
    if (strcasecmp(tag, "OK")==0) {
      parameter->get("filename", &(ui->PedFileName[0]));
      Unmanage("FileOpenDialog");
      parameter->get("action", &i);
      switch (i) {
      case 1:
	ui->AddDo(ReadSimul); break;
      case 2:
	ui->AddDo(WriteSimul); break;
      }
    }
    else {
      Unmanage("FileOpenDialog");
    }
    break;
  }
}

void NYI (Widget w, int *tag, unsigned long *reason)
{
 // Manage("nyiMessage");
}

void UnmanageProc(Widget w, int *tag, unsigned long *reason) {
 XtUnmanageChild(w);
}


RemoteControl::RemoteControl(GUI *interface) {
  ParameterDef d[] = { {"stoptime", ""}};
  parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  parameter->b = new bool[3]; parameter->nb = 3;
  parameter->b[0] = True;
  announce("construct RemoteControl...");

  ui = interface;
  // auto manage
  Manage("RemoteControl", this);
  announce("done\n");
}

//void RemoteControl::panel(Widget w, Widget par, char *tag) {
void RemoteControl::panelActivate(Widget w, Widget par, char *tag) {
  int nr = -1;
  bool ismenu = sscanf(tag, "%d", &nr) && nr >= 0;
  if (ismenu) {
    switch (nr) {
    case 22: /* Reset */
      ui->CF.reset();
      ui->AddDo(ResetSimul);
      ui->AddDo(UpdateDisplay);
      break;
	case 23: /* Calculate */
      ui->AddDo(CalculateSimul);
      break;
    case 41: // Scenery Plot
      ui->AddDo(DoTopViewPlot);
      break;
    case 51: /* window/new */
      ui->AddDo(OpenSzeneFenster);
      break;
    case 52: /* window/new */
      ui->AddDo(OpenStatFenster);
      break;
    case 53: /* window/new */
      ui->AddDo(OpenNetworkFenster);
      break;
    case 54: /* window/new */
      ui->AddDo(OpenHistoryFenster);
      break;
    case 58: // Mapper...
    case 59: // Save Window Settings
    default:
      Manage("nyiMessage");
      break;
    }
  }
  else if (strcasecmp(tag, "RUN")==0) {
    ui->CF.RunSimulation = 1;
  }
  else if (strcasecmp(tag, "HALT")==0) {
    ui->CF.RunSimulation = 0;
  }
  else if (strcasecmp(tag, "STEP")==0) {
    ui->CF.StepSimulation = 1;
    ui->CF.RunSimulation = 0; 
  }
  else if (strcasecmp(tag, "SNAP")==0) {
    ui->AddDo(Snap);
  }
  else if (strcasecmp(tag, "QUIT")==0) {
    ui->CF.Exit = 1;
  }
  else {
    Manage("nyiMessage");
  }
}

void RemoteControl::message(char *txt) {
  printf("%s\n", txt); 
}

// Viewer
ViewerControl::ViewerControl(GUI *interface) {
  ParameterDef d[] = { {"stoptime", ""}};
  parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
  parameter->b = new bool[3]; parameter->nb = 3;
  parameter->b[0] = True;
  announce("construct ViewerControl...");

  ui = interface;
  // auto manage
  Manage("ViewerControl", this);
  announce("done\n");
}

void ViewerControl::panelActivate(Widget w, Widget par, char *tag) {
  int nr = -1;
  bool ismenu = sscanf(tag, "%d", &nr) && nr >= 0;
  if (ismenu) {
    switch (nr) {
    case 41: // Scenery Plot
      ui->AddDo(DoTopViewPlot);
      break;
    default:
      Manage("nyiMessage");
      break;
    }
  }
  else if (strcasecmp(tag, "RUN")==0) {
    announce("RUN\n");
    ui->CF.RunSimulation = 1;
  }
  else if (strcasecmp(tag, "HALT")==0) {
    announce("HALT\n");
    ui->CF.RunSimulation = 0;
  }
  else if (strcasecmp(tag, "STEP")==0) {
    announce("STEP\n");
    ui->CF.StepSimulation = 1;
    ui->CF.RunSimulation = 0; 
  }
  else if (strcasecmp(tag, "BACK")==0) {
    announce("BACK\n");
    ui->CF.RunSimulation = 0;
    ui->AddDo(ViewBack);
  }
  else if (strcasecmp(tag, "RESET")==0) {
    ui->CF.RunSimulation = 0;
    ui->AddDo(ViewReset);
  }
  else if (strcasecmp(tag, "GOTO")==0) {
    ui->CF.RunSimulation = 0;
    ui->AddDo(ViewGoto);
  }
  else if (strcasecmp(tag, "QUIT")==0) {
    ui->CF.Exit = 1;
  }
  else {
    Manage("nyiMessage");
  }
}

void ViewerControl::message(char *txt) {
  printf("%s\n", txt); 
}


#ifdef _PARAM_HH_

void WithParameter::panel(int task, Widget w, Widget par, char *tag) {
  bool isarray, isbutton, isdef;
  int i = -1;
  ParameterValue *v;
  isarray = strlen(tag)>=3 && tag[1]=='.' && sscanf(tag+2, "%d", &i) && i>=0;
  char *equalsign = strchr(tag,'=');
  isdef = equalsign && *(equalsign+1)>'\0' ? 1 : 0;
  if (!isarray) {
    if (task==2) v = parameter->create(tag); // neuen Wert speichern
    else v = parameter->resolve(tag);
  }
  isbutton = !isarray && !v; 
  
  switch (task) {
  case 1: // create
    if (isbutton) return; // nothing todo
    else if (isarray) {
      switch (*tag) {
      case 'R': case 'r':
	if (i<parameter->na) {
	  char *txt = new char[20];
	  sprintf(txt, "%lf", parameter->a[i]);
	  SetSomething(w, XmNvalue, txt);
	  delete txt;
	}
	break;
      case 'V': case 'v':
	if (i<parameter->nv) {
	  char *txt = new char[30];
	  sprintf(txt, "%lf|%lf", parameter->v[i].x, parameter->v[i].y);
	  SetSomething(w, XmNvalue, txt);
	  txt;
	}
	break;
      case 'B': case 'b':
	if (i<parameter->nb) {
	  int set = parameter->b[i];
	  SetSomething(w, XmNset, (void*)set);
	}
	break;
      case 'F': case 'f':
	//if (i<sizeof(parameter->flags)*8) {
	if (i<sizeof(int)*8) {
	  v = parameter->resolve("flag");
	  if (v) {
	    int tmpflag = *v;
	    int set = (tmpflag& 1<<i) ? True : False;
	    SetSomething(w, XmNset, (void*)set);
	  }
	}
	break;
      }
    }
    else if (v) { // parameter
      char *txt;
      switch (v->type) {
      case _str:
	txt = new char[strlen(v->value.s)+1];
	if (v->value.s[0]=='\'') strcpy(txt, v->value.s + 1);
	else if (v->value.s[0]=='"') {
	  strcpy(txt, v->value.s + 1);
	  char *i = rindex(txt, '"');
	  if (i) *i = 0;
	}
	else strcpy(txt, v->value.s);
	break;
      case _real:
	txt = new char[64];
	sprintf(txt, "%.1lf", v->value.d);
	break;
      case _vec:
	txt = new char[64];
	sprintf(txt, "%.1lf|%.1lf", v->value.v->x, v->value.v->y);
	break;
      case _int:
	txt = new char[64];
	sprintf(txt, "%d", v->value.i);
	break;
      case _bool:
	txt = new char[6];
	if (v->value.b) strcpy(txt, "True");
	else strcpy(txt, "False");
	break;
      default:
	break;
      }
      SetSomething(w, XmNvalue, txt);
      delete txt;
    }
    else { // Fehler 
    }
    break;
 
  case 2: //value 
    if (isbutton) return; // nothing todo
    else if (isarray) {
      switch (*tag) {
      case 'R': case 'r':
	if (i<parameter->na) {
	  char *txt = NULL;
	  GetSomething(w, XmNvalue, &txt);
	  sscanf(txt, "%lf", &(parameter->a[i]));
	  if (txt) XFree(txt);
	}
	break;
      case 'V': case 'v':
	if (i<parameter->nv) {
	  char *txt = NULL;
	  GetSomething(w, XmNvalue, &txt);
	  sscanf(txt, "%lf|%lf", &(parameter->v[i].x), &(parameter->v[i].y));
	  if (txt) XFree(txt);
	}
	break;
      case 'B': case 'b':
	if (i<parameter->nb) {
	  Boolean set; 
	  GetSomething(w, XmNset, &set);
	  parameter->b[i] = set;
	  break;
	}
      case 'F': case 'f':
	//if (i<(sizeof(parameter->flags)*8)) {
	if (i<sizeof(int)*8) {
	  Boolean set; 
	  GetSomething(w, XmNset, &set);
	  v = parameter->create("flag");
	  int tmpflag = *v;
	  if (set) tmpflag |= 1<<i;
	  else tmpflag &= ~(1<<i);
	  *v = tmpflag;
	  break;
	}
      }
    }
    else if (v) { // parameter
      char *txt;
      if (!isdef) GetSomething(w, XmNvalue, &txt);
      else txt = equalsign+1;
      switch (v->type) {
      case _str:
	// v->value.s = new char[strlen(txt)+2];v->value.s[0] = '\'';
 	// strcpy(v->value.s+1,txt);
	char tt[256];
	int l = strlen(txt)+1;
	if (l>256) l = 256;
	strncpy(tt, txt, l);
	if (v->value.s) delete v->value.s;
	v->value.s = new char[l];
	strncpy(v->value.s, tt, l);
	break;
      default:
	v->scan(txt);
	break;
      }
      if (!isdef) XFree(txt);
    }
    else { // Fehler 
    }
    break;
    
  case 3: // activate
    if (isbutton) {
      init();
      test(printf("Activate `%s\'\n", tag));
      XtUnmanageChild(par);
    }
    break;
  }
}
#endif /* PARAMETER */

#ifdef _PLOT_HH_
void Plot::panelActivate(Widget w, Widget par, char *tag) {
  init();
  XtUnmanageChild(par);
  //XtDestroyWidget(par);
  if (strcasecmp(tag, "PLOT")==0) {
    g.ui->AddDo(DoPlot);
  }
  else if (strcasecmp(tag, "CANCEL")==0) {
    g.ui->AddDo(CancelPlot);
  }
  else {
    Manage("NotYet");
  }
}
#endif   



#endif // XWINDOWS




