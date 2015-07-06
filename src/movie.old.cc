#include "test.h"
#include "pedestrian.hh"
#include "ground.hh"
#include "floor.hh"
#include "movie.hh"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */

extern "C" off_t tell(int);

int write(int fd, char c) {
  char *b = new char;
  *b = c;
  int r = ::write(fd,b,1);
  delete b;
  return r;
}

Movie::Movie() {
  frame = NULL;
  MaxFrame = 0;
  movie_in = movie_out = -1;
  if (!parameter)
//    parameter = new Parameter(d, sizeof(d)/sizeof(ParameterDef));
    parameter = new Parameter();
}

Movie::~Movie() {
  MovieFileClose(&movie_in);
  MovieFileClose(&movie_out);
}

void Movie::init() {
  SimulObject::init();
  get(parameter, "n", &MaxFrame, 1000);
  if (MaxFrame) frame = new MovieFrame[MaxFrame];
  Nframe = 0;
  MovieFileClose(&movie_in);
  MovieFileClose(&movie_out);

  get(parameter, "record_rate", &record_rate, 0);
  get(parameter, "record_wait", &record_wait, 0.0);
  record_count=record_rate;

  get(parameter, "file_in", &movie_in_name, "" );

  get(parameter, "file_out", &movie_out_name, "" );
  get(parameter, "file_out_mode", &movie_out_mode, 0);
  movie_out_mode=(movie_out_mode)?O_APPEND:O_WRONLY|O_CREAT;

  MovieFileInit(&movie_in,movie_in_name,O_RDONLY);
  MovieFileInit(&movie_out,movie_out_name,movie_out_mode);

  // erster Schritt
  forw();
}

int Movie::ref(SimulObject *obj, SimulObjectType type)  {
  int good = 0;
  if (!obj) return good;
  switch (type) { 
  case _WALKER:
    walkers.append(obj);
    good = 1;
    break;
  case _FLOOR:
    floors.append(obj);
    good = 1;
    break;
  case _GROUND:
    grounds.append(obj);
    good=1;
    break;
  default:
    good = 0;
    break;
  }
  if (good) return 1;
  else return Walker::ref(obj, type);
}

int Movie::forw() {
  if (movie_in<0 ) return 0;
  indfrm++; // einen Schritt weiter;
  if (indfrm>=MaxFrame) {
    cout << "Movie::forw() indfrm>=MaxFrame\n" << endl;
    return 0;
  }
  int res;
  off_t current_pos;
  if (frame[indfrm].flag.valid) {
    // schon gelesen
    current_pos = lseek(movie_in, frame[indfrm].position, SEEK_SET);
    return mread(movie_in);
  }
  else {
    // neu
    Nframe = indfrm+1;
    current_pos = ::tell(movie_in);
    res=mread(movie_in);
    if (res>0) {  // ok, somthing read
      frame[indfrm].position = current_pos;
      frame[indfrm].T = root->T;
      frame[indfrm].flag.valid = 1;
    }
  }
  return res;
}

//int Movie::forw() {
//  if (movie_in<0 ) return 0;
//  indfrm++; // einen Schritt weiter;
//  if (indfrm>=MaxFrame) {
//    cout << "Movie::forw() indfrm>=MaxFrame\n" << endl;
//    return 0;
//  }
//  int res;
//  off_t current_pos;
//  if (frame[indfrm].flag.valid) {
//    // schon gelesen
//    current_pos = lseek(movie_in, frame[indfrm].position, SEEK_SET);
//    res = head.read(movie_in);
//  }
//  else {
//    // neu
//    Nframe = indfrm+1;
//    current_pos = ::tell(movie_in);
//    res = head.read(movie_in);
//    if (res>0) {  // ok, somthing read
//      frame[indfrm].position = current_pos;
//      frame[indfrm].T = head.T;
//      //      cout << float(head.T) << endl;
//      frame[indfrm].flag.valid = 1;
//    }
//  }
//  // if (!res) return 0;
//  return read();
//}

int Movie::backw() {
  off_t current_pos;
  int res = 0;
  indfrm--;
  if (indfrm>=0) {
    current_pos = lseek(movie_in, frame[indfrm].position, SEEK_SET);
    res = mread(movie_in);
  }
  return res;
}

//int Movie::backw() {
//  off_t current_pos;
//  int res = 0;
//  indfrm--;
//  if (indfrm>=0) {
//    current_pos = lseek(movie_in, frame[indfrm].position, SEEK_SET);
//    res = head.read(movie_in);
//  }
//  if (!res) return 0;
//  return read();
//}

int Movie::gotu(float time) {
  off_t current_pos;
  int res = 0;
  int i;
  for (i=0; i<Nframe; ++i) {
    if (frame[i].T >= time) break;
  }
  if (i<Nframe) {
    indfrm = i;
    current_pos = lseek(movie_in, frame[indfrm].position, SEEK_SET);
    res = mread(movie_in);
  }
  return res;
}

//int Movie::gotu(float time) {
//  off_t current_pos;
//  int res = 0;
//  for (int i=0; i<Nframe; ++i) {
//    if (frame[i].T >= time) break;
//  }
//  if (i<Nframe) {
//    indfrm = i;
//    current_pos = lseek(movie_in, frame[indfrm].position, SEEK_SET);
//    res = head.read(movie_in);
//  }
//  if (!res) return 0;
//  return read();
//}

int Movie::reset() {
  off_t current_pos;
  int res = 0;
  indfrm = 0;
  if (frame[indfrm].flag.valid) {
    current_pos = lseek(movie_in, frame[indfrm].position, SEEK_SET);
    res = mread(movie_in);
  }
  else {
    // neu
    Nframe = indfrm+1;
    current_pos = ::tell(movie_in);
    res = mread(movie_in);
    if (res>0) {  // ok, somthing read
      frame[indfrm].position = current_pos;
      frame[indfrm].T = root->T;
      frame[indfrm].flag.valid = 1;
    }
  }
  return res;
}

//int Movie::reset() {
//  off_t current_pos;
//  int res = 0;
//  indfrm = 0;
//  if (frame[indfrm].flag.valid) {
//    current_pos = lseek(movie_in, frame[indfrm].position, SEEK_SET);
//    res = head.read(movie_in);
//  }
//  else {
//    // neu
//    Nframe = indfrm+1;
//    current_pos = ::tell(movie_in);
//    res = head.read(movie_in);
//    if (res>0) {  // ok, somthing read
//      frame[indfrm].position = current_pos;
//      frame[indfrm].T = float(head.T)/10000.0;
//      frame[indfrm].flag.valid = 1;
//    }
//  }
//  if (!res) return 0;
//  return read();
//}


int Movie::snap(SnapTyp whence) {
  if (movie_out!=-1)
    switch (whence) { 
    case immediate:
      mwrite(movie_out);
      break;
    case TimeStep:
      if( root->T >= record_wait) {
	if (! (--record_count)) {
	  record_count=record_rate;
	  mwrite(movie_out);
        }
      }
      break;
    default:
      cerr<<__FILE__<<__LINE__
		<<" Movie::snap: Internal error, invalid argument\n";
    }
}

int Movie::mwrite(int fd)
{
  /*
   * Write snapshot to file.
   *
   * Structure is as follows:
   *  Field          Size     Contents
   *  =====          ====     ========
   *  preamble       1        PreSnapShotID
   *  time           4        timestamp
   *  channels       2        # of channels in this record
   *  
   *  SimObjTyp#1    2        Type of the next simul objects
   *  SimObjNum#1    2        Number of simul objects of type SimObjTyp
   *    SimObj#1.1   unknown  dataformat known by appropriate simul object)
   *    SimObj#1.2   unknown
   *    [...]
   *    SimObj#1.SimObjNum#
   *  postamble      1        PostSimObjID
   *
   *  SimObjTyp#2    2
   *  SimObjNum#2    2
   *    [...]
   *  postamble      1        PostSimObjID
   *
   *  [...]
   *
   *  SimObjTyp#channels
   *  SimObjNum#channels
   *    [...]
   *  postamble      1        PostSnapShotID
   */

  int channels;


  register int i;
  char tmpch;
  int rc=0;


  //  rc+=(fd==-1)?1: ::write(fd,&(tmpch=PreSnapShotID),1);
  // tmpch=PreSnapShotID;
  rc+=(fd==-1)?1: ::write(fd, PreSnapShotID);
  rc+=(fd==-1)?4:write_netbor4(fd,root->T)<<2;

  //count channels
  channels=(walkers.N()?1:0)+(grounds.N()?1:0)+(floors.N()?1:0);

  rc+=(fd==-1)?2:write_netbor2(fd,channels)<<1;

  //write out all ground objects
  Pointer<Ground> gro;
  if (grounds.N()) rc+=::write_object(fd,_GROUND, gro, grounds);

  //write out all walkers
  Pointer<Walker> wlk;
  if (walkers.N()) rc+=::write_object(fd,_WALKER,wlk,walkers);

  //write out all floors
  Pointer<Floor> flo;
  if (floors.N()) rc+=::write_object(fd,_FLOOR,flo,floors);
  

  // write Postamble of this snapshot:
  //tmpch=PostSnapShotID;
  rc+=(fd==-1)?1: ::write(fd, PostSnapShotID);
  //   tmpch = PostSnapShotID;
  //   rc+=(fd==-1)?1: ::write(fd,&tmpch,1);

  return rc;
}

int Movie::mread(int fd, int num)
{

  int channels,tmp,SimObjTyp,SimObjNum;
  unsigned char c;
  register int i,j;
  int rc=0;

  rc+=::read(fd,&c,1);
  
  if (rc==-1 || rc==0)
  {
    return rc;		//return error or EOF
  }
  
  
  if (c!=PreSnapShotID)
  {
    cerr<<__FILE__" "<<__LINE__<<": Movie::read(), unrecoverable error:"<<endl
      <<"PreSnapShotID missing at beginning of record."<<endl
	<<"Read "<<(int)c<<" expected "<<PreSnapShotID<<endl
	  <<"at position "<<tell(fd);
    return -1;		//return error
  }
  
  rc+=read_netbor4(fd,&root->T)<<2;
  //cerr<<"("<<root->T<<")";
  
  //read number of channels
  rc+=read_netbor2(movie_in,&channels)<<1;
  //cerr<<"Processing "<<channels<<" channel(s)." <<endl;
  
  //read in all channels
  for (i=1; i<=channels; ++i)  {
    rc+=read_netbor2(fd,&SimObjTyp)<<1;
    rc+=read_netbor2(fd,&SimObjNum)<<1;
    switch (SimObjTyp) {
    case _WALKER:
    {
      Pointer<Walker>wlk;
      //cerr<<"Processing "<<SimObjNum<<" walkers."<<endl;
      rc+=::read_object(fd,_WALKER,&wlk,&walkers,SimObjNum);
    }
    break;
  case _GROUND:
  {
    Pointer<Ground>gro;
    //cerr<<"Processing "<<SimObjNum<<" grounds, "<<endl;
    rc+=::read_object(fd,_GROUND,&gro,&grounds,SimObjNum);
  }
    break;
    case _FLOOR:
  { 
    Pointer<Floor>flo;
    //cerr<<"Processing "<<SimObjNum<<" Floors, "<<endl;
    rc+=::read_object(fd,_FLOOR,&flo,&floors,SimObjNum);
  }
    break;
  default:
  {
    List DummyList;
    //cerr<<"Discarding "<<SimObjNum<<" objects of type "<<SimObjTyp<<"."<<endl;
    Pointer<Walker>dummy;	//any type already used
    rc+=::read_object(fd,_DUMMY,&dummy,&DummyList,SimObjNum);
  }
  }
  } // end for
  
  //read Postamble of this snapshot:
  rc+=::read(fd,&c,1);
   if (c!=(unsigned char)PostSnapShotID)
   {
   cerr<<__FILE__" "<<__LINE__<<": Movie::read(), unrecoverable error:"<<endl
       <<"PostSnapShotID missing at end of record."<<endl
       <<"Read "<<(int)c<<" expected "<<(int)PostSnapShotID<<"."<<endl
       <<"At position "<<tell(fd)<<endl;
   }

  return rc;
}

//int Movie::read() {
 // Pointer<Ground> gro;
//  Pointer<WalkerPop> pop;
//  int res = 0;
//  if (head.N>SimulRecordHeaderMaxChannels) head.N=SimulRecordHeaderMaxChannels;
//  for (int i=0; i<head.N; ++i ) {
//    //printf("read frame %d, type %d\n", i, head.type[i]);
//    switch (head.type[i]) {
//    case _WALKER_POP:
//      
//      if (!subpops.N()) {
//	//cout << "new WalkerPop" << endl;
//	WalkerPop *wp = new WalkerPop;
//	//	wp->init();
//	subpops.append(wp);
//	res += wp->read(movie_in);
//      }
//      else {
//	if (!pop) pop = subpops;
//	else ++pop;
//	res += pop->read(movie_in);
//      }
//      break;
//    case _GROUND:
//      if (!grounds.N()) grounds.append(new Ground);
//      if (!gro) gro = grounds;
//      else ++gro;
//      res += gro->read(movie_in);
//      break;
//    }
//  }
//  return res;
//}

int Movie::MovieFileInit(int* fd, char* MovieFileName, int mode)
  {
	int i,rc=0;
	char* creator;

	if (*fd>-1) MovieFileClose (fd);
	if (!*MovieFileName) return(-1);

	switch (mode)
	{
	case O_RDONLY:
	case O_RDWR:
  	  *fd = open(MovieFileName, mode, 0640);
	  if (*fd==-1) {perror("movie.cc 449: Movie::MovieFileInit");return -1;}

	  read_netbor4(*fd,&i);
	  if (i!=MovieFileID)
	  {
	    cerr<<"Moviefile "<<MovieFileName<<" has invalid identifier."
	      <<endl<<"It will be ignored."<<endl;
	    close (*fd);
	    *fd=-1;
	    return -1;
	  }

	  read_netbor2(*fd,&i);
	  if ((i>>8)!=VersionMajorID)
	  {
	    cerr<<"Moviefile "<<MovieFileName<<" is a version "<<(i>>8)
	      <<" File."<<endl
	      <<"Need Version"<<VersionMajorID<<". The file will be ignored."
	      <<endl;
	    close (*fd);
	    *fd=-1;
	    return -1;
	  }
	  if ((i & 0xff)>VersionMinorID)
	  {
	    cerr<<"Moviefile "<<MovieFileName<<" is a version "<<(i>>8)
	      <<'.'<<(i&0xff)<<" File."<<endl
	      <<"Need Version minor number "<<VersionMinorID
	      <<"or below. The file will be ignored."
	      <<endl;
	    close (*fd);
	    *fd=-1;
	    return -1;
	  }

	  //discard patchlevel.
	  read_netbor2(*fd,&i);
	  //discard creation date
	  read_netbor4(*fd,&i);
	  //discard creator
	  unsigned char creator_len;
	  ::read(*fd,&creator_len,1);
	  creator = new char[creator_len];
	  i = ::read(*fd,creator,creator_len);
	  if (i==EOF)
	  {
	    cerr<<"Unexpected end of file in "<<MovieFileName<<'.'<<endl
	      <<"The File will be ignored."<<endl;
	    close(*fd);
	    *fd=-1;
	    return -1;
	  }
	  creator[i]='0';
	  delete [] creator;

	  return (i+13); // i contains length of creator, 13 is the rest.

        case O_WRONLY:
	case O_WRONLY|O_CREAT:
  	  *fd = open(MovieFileName, mode, 0640);
	  if (*fd==-1) {
	    perror("movie.cc 508: Movie::MovieFileInit");return -1;
	  } 

	  //write ID
	  write_netbor4(*fd,(int)MovieFileID);

	  //write Version Number
	  write_netbor2(*fd,(int)VersionID);

	  //write patchlevel.
#ifndef PATCHLEVEL
#define PATCHLEVEL 0
#endif
	  write_netbor2(*fd,(int)PATCHLEVEL);
	
	  //write creation date
	  i= ::time(NULL);
	  write_netbor4(*fd,i);
 
	  //write creator
	  char tmpch;
	  //	i = ::write(*fd,&(tmpch=0),1);	// length=0, no creator data
	  tmpch=0;
	  i = ::write(*fd, 0);	// length=0, no creator data
	  if (i == -1)
	  {
	    cerr<<"Write error on file "<<MovieFileName<<'.'<<endl
	      <<"File ignored."<<endl;
	    *fd=-1;
	    return -1;
	  }

	  return (13); // 13 bytes written. (no creator)
	case O_APPEND:

	  //check if it is a movie file:
	  MovieFileInit(fd,MovieFileName,O_RDWR);
	  if (*fd=-1) return -1;	//no, it's not!

	  lseek(*fd,-2,SEEK_END);
	  char c[1];
	  ::read(*fd,&c,2);
	  if ((c[0]!=(unsigned char)PostSimObjID) 	\
		|| (c[1]!=(unsigned char)PostSnapShotID))
	  {
	    cerr<<"Moviefile "<<MovieFileName<<" opened for append"<<endl
	      <<"doesn't end with char pair PostSimObjID, PostSnapShotID."
	      <<endl<<"Expected "<<(int)PostSimObjID<<','
	      <<(int)PostSnapShotID<<", read "<<(int)c[0]<<','<<(int)c[1]
	      <<endl<<"File ignored"<<endl;
	    close(*fd);
	    *fd=-1;
	    return -1;
	  }

	  //write new date.
	  lseek(*fd,8,SEEK_SET);
	  i = ::time(NULL);
	  write_netbor4(*fd,i);

	  //seek to logical end of file
	  if ( -1 == lseek(*fd,0,SEEK_END) )
	  {
	    close(*fd);
	    *fd=-1;
	    return -1;
	  };

	  return 0;	// claim to have 0 bytes written.
	default:
	  return -1;	//invalid mode.
	}
  }

int Movie::MovieFileClose(int  *fd)
  {
    if (*fd<=-1) return 0;
    close(*fd);
    *fd=-1;
  }

int Movie::appendfile(unsigned int k) {
  if (movie_out<0) MovieFileInit(&movie_out,movie_out_name, O_APPEND);
 
  int res = 0;
  lseek(movie_in, frame[indfrm].position, SEEK_SET);
  if (k<=0 && indfrm<Nframe-1) {
    k = frame[indfrm+1].position - frame[indfrm].position;
  }
  char *buff = new char[k];
  k = ::read(movie_in, buff, k);
  res += ::write(movie_out, buff, k); 
  delete [] buff;
  return res;
}
