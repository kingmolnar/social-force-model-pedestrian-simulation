#include "test.h"
#include <stdlib.h>
#include <iostream.h>
#include <sys/types.h>
#include <stdio.h>
#include "vector.hh"
#include "list.hh"
#include "mall.hh"
#include "street.hh"
#include "pedestrian.hh"

#ifndef NULL
#define NULL ((void*)0)
#endif

main () {
  Mall *mall = new Mall;
  ParameterDef mdef[] = { {"tau", "0.2"}
			};
  mall->parameter->insert(mdef, sizeof(mdef)/sizeof(ParameterDef));

  Street *street = new Street;
  ParameterDef sdef[] = {};
  street->parameter->insert(sdef, sizeof(sdef)/sizeof(ParameterDef));
  
  

