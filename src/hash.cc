#include <stdlib.h>
#include "hash.hh"

const int HashVecSize = 20;

HASH::HASH() {
  N = 500;
  table = new HASH_TABLE_STRUCT[N+1]; 
}

HASH::HASH(int n) {
  N = n;
  table = new HASH_TABLE_STRUCT[N+1]; 
}

HASH::~HASH() {
  for (int i=0; i<N; ++i) {
    if (table[i].name) delete table[i].name;
  }
  delete table;
}

int HASH::regist (char *token, void *id) {
  int ndx;
  for (ndx = function(token); 
       ((table[ndx].name != NULL) && (ndx < N));
       ndx++);
  if (table[ndx].name != NULL)
    for (ndx = 0;
	 ((table[ndx].name != NULL) && (ndx < N));
	 ndx++);
  if (ndx == N) return 0;
  else {
    table[ndx].name = new char[strlen(token) + 1];
    strcpy(table[ndx].name, token);
    table[ndx].id = id;
    return 1;
  }
}


int HASH::lookup (char *token, void **id) {
  int ndx;
  for (ndx = function(token);
       ((table[ndx].name != NULL) && (ndx <= N));
       ndx++)
    if (strcmp(token, table[ndx].name) == 0) {
      *id = table[ndx].id;
      return 1;
    }

  if (ndx > N)
    for (ndx = 0;
	 ((table[ndx].name != NULL) && (ndx <= N));
	 ndx++)
      {
	if (strcmp(token, table[ndx].name) == 0) {
	  *id = table[ndx].id;
	  return 1;
	}
      }
  
  return 0;
}

  
int HASH::function (char *token) {
  typedef union {
    short int		intname[HashVecSize];	 /* name as vector of ints */
    char		charname[2*HashVecSize]; /* name as vector of chars */
  } HASHName;
  HASHName		locname;	/* aligned name */
  int			namelen;	/* length of name */
  int			namelim;	/* length limit (fullword size) */
  int			namextra;	/* limit factor remainder */
  int			code = 0;	/* HASH code value */
  int			ndx;		/* loop index */
  
  /*
   * Copy the name into the local aligned union.
   * Process the name as a vector of integers, with some remaining characters.
   * The string is copied into a local union in order to force correct
   * alignment for alignment-sensitive processors.
   */
  strcpy (locname.charname, token);
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
  
  return (code&0x7FFF) % N;
}



