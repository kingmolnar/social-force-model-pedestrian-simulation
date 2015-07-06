#ifndef _MOVIE_HH_
#define _MOVIE_HH_

#include "simul.hh"
#include "graphic.hh"
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

extern "C" {
  //  int time(int *);
  off_t tell(int);
}

enum SnapTyp{
  TimeStep=0,
  immediate
};

struct MovieFrame {
  off_t position;
  off_t length;
  float T;
  union {
    struct {
      unsigned int valid : 1;
      unsigned int read  : 1;
      unsigned int written : 1;
      unsigned int marked : 1;
      unsigned int start : 1;
      unsigned int stop : 1;
    } flag;
    char byteflag;
  };
  MovieFrame() {position = length = 0; T = 0.0; byteflag = 0;} 
};

//inline 
int write(int fd, char c);

class Movie: public SimulObject {
private: 
//  SimulRecordHeader head;
  MovieFrame *frame;
  int Nframe;
  int MaxFrame;
  int indfrm;
  int movie_in;
  int movie_out;
  int movie_out_mode;
  int record_count;
  int record_rate;
  double record_wait;
  
  int MovieFileInit(int*, char*, int);
  int MovieFileClose(int*);
  //  off_t current_pos;
  char* movie_in_name;
  char* movie_out_name;
public:
  List grounds;
  List walkers;
  List floors;
  List blabla;//dummy list
  List subpops; //obsolete

  Movie();
  ~Movie();
  SimulObjectType objecttype(int i = 0) {
    return i==0 ? _MOVIE: SimulObject::objecttype(--i);
  } 
  void init();
  int ref(SimulObject*, SimulObjectType);
  int forw();
  int backw();
  int gotu(float);
  int reset();
  int mread(int,int=0);
  int snap(SnapTyp);
  int mwrite(int);
  void setmarker() {
    if (frame[indfrm].flag.valid) frame[indfrm].flag.marked = 1;
  }
  void delmarker() {frame[indfrm].flag.marked = 0;}
  int ismarked() {return frame[indfrm].flag.marked;}
  int appendfile(unsigned int k = 0);
  float T() {return frame[indfrm].T;}
  bool movieIn() { return movie_in!=-1; }
  bool movieOut() { return movie_out!=-1; }
};

// auxilliary function:
template<class Type>
int read_object(int fd, SimulObjectType SimObjTyp,
			       Pointer<Type> *ObjPtr,
			       List *ObjList,
			       int count)
{
  register int i;
  Pointer<Type> tmpPtr;
  int rc=0,tmp,obj_length;

  //read [count] objects of type <Type>

  for (i=1; i<=count; ++i)
  {
    rc+=read_netbor4(fd,&obj_length)<<2;
    if (SimObjTyp==_DUMMY)
    {
      char *tmpbuf=new char[obj_length];
      rc+=::read(fd,tmpbuf,obj_length);
      delete [] tmpbuf;
    }
    else
    {
      //create element if list empty;
      if (!ObjList->N()) ObjList->append(new Type);

      // point to first element, if we don't point anywhere so far
      if (! (*ObjPtr)) *ObjPtr=*ObjList;

      // otherwise point to next element:
      else
      {
        tmpPtr=*ObjPtr;
        // create one if there is no next element:
        if(! (++tmpPtr)) ObjList->append(new Type);
        ++(*ObjPtr);
      }
      rc+=tmp=(*ObjPtr)->mread(fd,obj_length);
      // here should be some code to adjust the filepointer
      // if less than obj_length bytes had been read.
      if(obj_length-tmp) {
        // read pending bytes:
        char* buf;
        buf = new char [obj_length-tmp];
        rc += ::read(fd,buf,obj_length-tmp);
        delete [] buf;
        //  cerr<<"Adjusting "<<obj_length-tmp<<" pending bytes."<<endl;
      }
    }
  } 
  //read postamble
  unsigned char c;
  (rc+=read(fd,&c,1));
   if (c!=PostSimObjID)
   {
     cerr<<__FILE__" "<<__LINE__<<" ::read_object unrecoverable Error:"<<endl
       <<"Lost synchronization after read of SimObject Type "<<SimObjTyp
       <<'.'<<endl<<"Expected "<<PostSimObjID<<", read"<<c<<endl
       <<"at position "<< tell(fd)
       <<endl;
     exit (-1);
   }
  return(rc);
}

// auxilliary function:
template<class Type>
int write_object(int fd, SimulObjectType SimObjTyp,
			       Type dummy,
			       List &ObjList)
{
  register int i;
  Type ObjPtr;
  int rc=0, SimObjSiz;

  //write out all [id] objects

  rc+= (fd==-1)?2: write_netbor2(fd,(int*)&SimObjTyp)<<1;
  //write SimObjNum
  rc+= (fd==-1)?2: write_netbor2(fd,ObjList.N())<<1;

  //write SimObjNum objects
  for (i=1, ObjPtr=ObjList; ObjPtr && (i<=ObjList.N()); ++ObjPtr, ++i)
  {
    // mwrite(-1) may not write anything physically,
    // but returns size in bytes of record that would be written.
    // so, write this recordsize to disk now, then the record itself.
    rc+= (fd==-1)?4: write_netbor4(fd,SimObjSiz=ObjPtr->mwrite(-1))<<2;
    rc+=ObjPtr->mwrite(fd);

    // here should be some code to pad with zeroes or sth if less
    // than SimObjSiz bytes had been written.

  }
   if (ObjPtr || (i<=ObjList.N()))
   {
     cerr<<__FILE__" "<<__LINE__<<" unrecoverable internal error."<<endl
 	<<"After looping through an object list, the obj pointer must be"<<endl
 	<<"NULL, and i must be the number of objects ObjList.N()"<<endl
 	  //	<<"ObjectID: ["<<SimObjTyp<<"], ObjPtr: ["<<ObjPtr<<"] ,"<<endl
 	<<" i: ["<<i<<"] , ObjList.N(): ["<<ObjList.N()<<"] ."<<endl
 	<<"Program aborted."<<endl;
     exit(-1);
   }

  //write postamble
  /* return (rc+= (fd==-1)?1: ::write(fd,(char*) &PostSimObjID,1));*/
  
  //PM   unsigned char c=PostSimObjID;
  //  char c=PostSimObjID;  rc+=::write(fd,&c,1);
  rc+=::write(fd, PostSimObjID);
}

enum FileIDs{ /* redundancy check IDs --- only one byte long */
     SyncID=0,       /* for various synchronisations */
     PreSnapShotID,
     PostSimObjID,
     PostSnapShotID,
     /* information holding IDs --- 4 bytes */
     MovieFileID=0x0b0a0d10,     /* PMJK in l_e */
     VersionMajorID=1,
     VersionMinorID=01,
     VersionID=VersionMinorID+(VersionMajorID<<8)
   };
#endif
