#define NTEST
#include "test.h"
#include <stdlib.h>
#include <iostream.h>
#include <sys/types.h>
#include <unistd.h>

#include "vector.hh"
#include "list.hh"
#include "simul.hh"
#include "floor.hh"
#include "street.hh"
#include "pedestrian.hh"
#include "ground.hh"
#include "movie.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>     /* definition of OPEN_MAX */


main (unsigned int argc, char **argv) {
  pid_t pid;
  char text[20];

  Movie *movie();
  ParameterDef md[] = { {"filename", "'Filme/cross100b.movie"};
		       {"n", "1000"}
		     };
  movie->parameter->insert(md, sizeof(md)/sizeof(ParameterDef));
  movie->init();
}















