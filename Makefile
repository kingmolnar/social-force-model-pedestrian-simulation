# CAUTION: This is an older version of the Makefile ... not tested on current code


CINCLUDES =	-I.\
		-I.. \
		-I$(TOP)/usr/include/MIT \
		-I$(TOP)/lib \
		-I$(TOP)/lib/Xt 

LINTFLAGS=	$(CINCLUDES)

LDFLAGS = 	-L$(TOP)/lib/Mrm \
		-L$(TOP)/lib/DXm \
		-L$(TOP)/lib/Xm \
		-L$(TOP)/lib/Xt 


MAINHEADER=	list.hh vector.hh param.hh simul.hh simultypes.hh hash.hh \
		arrlst.hh evolution.hh endian.hh custom.h
XMAINHEADER =  ${MAINHEADER} fenster.hh szfenster.hh
DESTDIR=
UIL = ${DESTDIR}/usr/bin/dxuil

CC = cc
CPLUS = time g++


CFLAGS = -I. -I${DESTDIR}/usr/include/X11 

LIBS = ${DESTDIR}/usr/lib/libdwt.a  ${DESTDIR}/usr/lib/libX11.a
LIBWEG = -lc -lDXm -L/usr/new/gnu/lib -lg++ -limsl
rem_LIBOPT = -limsl -lm
LIBOPT =  -lm
LIBZUS = ${LIBOPT}  -lMrm -lXm -lXt -lX11

OPT = -g 
rem_OPT = -g -DIMSL

depend: 
	makedepend -DMOTIF -o Xpedestrain.o  pedestrian.cc 

xsim.uid: xsim.uil
	uil -o xsim.uid xsim.uil
	
xx.uid: xx.uil
	uil -o xx.uid xx.uil
	
pedestrian.o: pedestrian.cc pedestrian.hh street.hh floor.hh ground.hh \
	graphic.hh	${MAINHEADER}	 
	${CPLUS} -c ${OPT}  pedestrian.cc

pedestrian_hp.o: pedestrian.cc pedestrian.hh street.hh floor.hh ground.hh \
	graphic.hh	${MAINHEADER}	 
	${CPLUS} -o pedestrian_hp.o -c ${OPT}  pedestrian.cc


street.o: street.cc street.hh floor.hh graphic.hh pedestrian.hh \
		${MAINHEADER}
	${CPLUS} -c ${OPT} street.cc

street_hp.o: street.cc street.hh floor.hh graphic.hh pedestrian.hh \
		${MAINHEADER}
	${CPLUS} -o street_hp.o -c ${OPT} street.cc


attraction.o: attraction.c attraction.h street.h floor.h \
		fenster.h xsim.h
	${CC} -c ${OPT} attraction.c

ground.o: ground.cc ground.hh floor.hh \
		${MAINHEADER}
	${CPLUS} -c ${OPT} ground.cc

ground_hp.o: ground.cc ground.hh floor.hh \
		${MAINHEADER}
	${CPLUS} -o ground_hp.o -c ${OPT} ground.cc

floor.o: floor.cc floor.hh pedestrian.hh street.hh ground.hh graphic.hh \
		${MAINHEADER}
	${CPLUS} -c ${OPT} floor.cc

floor_hp.o: floor.cc floor.hh pedestrian.hh street.hh ground.hh graphic.hh \
		${MAINHEADER}
	${CPLUS} -o floor_hp.o -c ${OPT} floor.cc


fenster.o: fenster.cc  ${XMAINHEADER}
	${CPLUS} -c -DMOTIF ${OPT} -I/usr/include/MIT -I/usr/include fenster.cc
	
fenster_hp.o: fenster.cc  ${XMAINHEADER}
	${CPLUS} -o fenster_hp.o \
	-c -DMOTIF ${OPT} fenster.cc

szfenster.o: szfenster.cc floor.hh pedestrian.hh street.hh ground.hh \
 		${XMAINHEADER}
	${CPLUS} -c -DMOTIF \
		${OPT} -I/usr/include/MIT  szfenster.cc

szfenster_hp.o: szfenster.cc floor.hh pedestrian.hh street.hh ground.hh \
 		${XMAINHEADER}
	${CPLUS} -o szenefenster_hp.o -c -DMOTIF \
		${OPT} szfenster.cc

list.o: list.cc list.hh
	${CPLUS} -c ${OPT} list.cc

list_hp.o: list.cc list.hh
	${CPLUS} -o list_hp.o -c ${OPT} list.cc

hash.o: hash.cc hash.hh
	${CPLUS} -c ${OPT}  hash.cc

endian.o: endian.cc endian.hh custom.h
	${CPLUS} -c ${opt} endian.cc

endian_hp.o: endian.cc endian.hh custom.h
	${CPLUS} -o endian_hp.o -c ${opt} endian.cc

random.o: random.cc random.hh
	${CPLUS} -c  ${OPT}  random.cc

param.o: param.cc ${MAINHEADER}
	${CPLUS} -c ${OPT} param.cc

param_hp.o: param.cc ${MAINHEADER}
	${CPLUS} -o param_hp.o -c ${OPT} param.cc

simul.o: simul.cc floor.hh street.hh pedestrian.hh ground.hh ${MAINHEADER}
	${CPLUS} -c ${OPT} simul.cc

simul_hp.o: simul.cc floor.hh street.hh pedestrian.hh ground.hh ${MAINHEADER}
	${CPLUS} -o simul_hp.o -c ${OPT} simul.cc

movie.o: movie.cc movie.hh pedestrian.hh ground.hh ${MAINHEADER}
	${CPLUS} -c ${OPT} movie.cc

movie_hp.o: movie.cc movie.hh pedestrian.hh ground.hh ${MAINHEADER}
	${CPLUS} -o movie_hp.o -c ${OPT} movie.cc

ui.o: gui.cc gui.hh ${MAINHEADER} hash.hh
	${CPLUS} -o ui.o -c ${OPT} -I/usr/include/MIT gui.cc

ui_hp.o: gui.cc gui.hh ${MAINHEADER} hash.hh
	${CPLUS} -o ui_hp.o -c ${OPT} gui.cc

gui.o: gui.cc gui.hh ${MAINHEADER} hash.hh
	${CPLUS} -c ${OPT} -DXWINDOWS -I/usr/include/MIT gui.cc

gui_hp.o: gui.cc gui.hh ${MAINHEADER} hash.hh
	${CPLUS} -o gui_hp.o -c ${OPT} -DXWINDOWS gui.cc


route.o: route.cc route.hh street.hh floor.hh pedestrian.hh ${MAINHEADER}
	${CPLUS} -c ${OPT} route.cc

Xroute.o: route.cc route.hh street.hh floor.hh pedestrian.hh ${XMAINHEADER}
	${CPLUS} -o Xroute.o -DMOTIF -c ${OPT} route.cc

XUroute.o: route.cc route.hh street.hh floor.hh pedestrian.hh ${XMAINHEADER}
	${CPLUS} -o Xroute.o -DMOTIF -DUNIRAS -c ${OPT} route.cc

plot.o: plot.cc plot.hh ${XMAINHEADER} floor.hh street.hh pedestrian.hh \
	movie.hh
	${CPLUS} -c -g -DMOTIF -DUNIRAS plot.cc
psplot.o: psplot.cc psplot.hh ${XMAINHEADER} floor.hh street.hh pedestrian.hh \
	movie.hh
	${CPLUS} -c -g -DMOTIF  psplot.cc

replaceMOTIF.o: replaceMOTIF.cc pedestrian.hh ground.hh street.hh
	${CPLUS} -c -g ${OPT} replaceMOTIF.cc

replaceMOTIF_hp.o: replaceMOTIF.cc pedestrian.hh ground.hh street.hh
	${CPLUS} -o replaceMOTIF_hp.o -c -g ${OPT} replaceMOTIF.cc

replaceUNIRAS.o: replaceUNIRAS.cc pedestrian.hh ground.hh street.hh
	${CPLUS} -c -g ${OPT} replaceUNIRAS.cc

replaceUNIRAS_hp.o: replaceUNIRAS.cc pedestrian.hh ground.hh street.hh
	${CPLUS} -o replaceUNIRAS_hp.o -c -g ${OPT} replaceUNIRAS.cc

forcemodel.o: forcemodel.cc floor.hh pedestrian.hh street.hh ground.hh gui.hh \
	${MAINHEADER}
	${CPLUS} -c ${OPT} forcemodel.cc 	

forcemodel_hp.o: forcemodel.cc floor.hh pedestrian.hh street.hh ground.hh \
	gui.hh 	${MAINHEADER}
	${CPLUS} -o forcemodel_hp.o -c ${OPT} forcemodel.cc 	


forcemodel: forcemodel.o floor.o pedestrian.o street.o ground.o \
	simul.o param.o list.o ui.o movie.o evolution.o \
	replaceMOTIF.o replaceUNIRAS.o endian.o
	g++ -o forcemodel forcemodel.o floor.o pedestrian.o street.o ground.o \
	simul.o param.o list.o ui.o movie.o evolution.o \
	replaceMOTIF.o replaceUNIRAS.o endian.o ${LIBOPT}

force_hp: forcemodel_hp.o floor_hp.o pedestrian_hp.o street_hp.o ground_hp.o \
	simul_hp.o param_hp.o list_hp.o ui_hp.o movie_hp.o evolution_hp.o \
	replaceMOTIF_hp.o replaceUNIRAS_hp.o endian_hp.o
	g++ -o force_hp forcemodel_hp.o floor_hp.o pedestrian_hp.o \
	street_hp.o ground_hp.o simul_hp.o param_hp.o list_hp.o ui_hp.o \
	movie_hp.o evolution_hp.o \
	replaceMOTIF_hp.o replaceUNIRAS_hp.o endian_hp.o ${LIBOPT}

xx.o: xx.cc floor.hh pedestrian.hh street.hh gui.hh ground.hh ${XMAINHEADER}
	${CPLUS} -g -c -DMOTIF  -I/usr/include/MIT xx.cc

xx_hp.o: xx.cc floor.hh pedestrian.hh street.hh gui.hh ground.hh ${XMAINHEADER}
	${CPLUS} -o xx_hp.o -g -c -DMOTIF  xx.cc

xu.o: xx.cc floor.hh pedestrian.hh street.hh ground.hh \
	gui.hh ${XMAINHEADER} plot.hh 
	${CPLUS} -o xu.o -g -c -DMOTIF -DUNIRAS -I/usr/include/MIT xx.cc

viewer.o: viewer.cc floor.hh pedestrian.hh street.hh gui.hh ground.hh \
	movie.hh ${XMAINHEADER} plot.hh 
	${CPLUS} -g -c -DMOTIF -DUNIRAS -I/usr/include/MIT viewer.cc
		
sim:	xu.o floor.o pedestrian.o street.o ground.o movie.o evolution.o \
	simul.o param.o list.o fenster.o szfenster.o gui.o plot.o hash.o \
	endian.o 	
	time link -g -o sim xu.o floor.o pedestrian.o street.o ground.o \
	simul.o param.o list.o fenster.o szfenster.o gui.o hash.o plot.o \
	movie.o evolution.o endian.o ${LIBZUS}

xim:	xx.o floor.o pedestrian.o street.o ground.o movie.o evolution.o \
	simul.o param.o list.o fenster.o szfenster.o gui.o plot.o hash.o \
	endian.o 	
	${CPLUS} -g -o xim xx.o floor.o pedestrian.o street.o ground.o \
	simul.o param.o list.o fenster.o szfenster.o gui.o hash.o replaceUNIRAS.o \
	movie.o evolution.o endian.o ${LIBZUS}
	 
sim_hp:	xx_hp.o floor_hp.o pedestrian_hp.o street_hp.o ground_hp.o \
	movie_hp.o evolution_hp.o simul_hp.o param_hp.o list_hp.o \
	fenster_hp.o szfenster_hp.o gui_hp.o plot_hp.o hash_hp.o \
	endian_hp.o replaceUNIRAS_hp.o
	time ${CPLUS} -g -o sim_hp xx_hp.o floor_hp.o pedestrian_hp.o \
	street_hp.o ground_hp.o simul_hp.o param_hp.o list_hp.o \
	fenster_hp.o szfenster_hp.o gui_hp.o hash_hp.o \
	movie_hp.o evolution_hp.o endian_hp.o replaceUNIRAS_hp.o ${LIBZUS}
	 
viewer:	viewer.o floor.o pedestrian.o street.o ground.o movie.o evolution.o \
	simul.o param.o list.o fenster.o szfenster.o gui.o hash.o \
	plot.o endian.o 
	time link -g -o viewer viewer.o floor.o pedestrian.o street.o \
	ground.o simul.o param.o list.o fenster.o szfenster.o gui.o hash.o \
	movie.o evolution.o plot.o endian.o ${LIBZUS} 
#	time ${CPLUS} -g -o viewer viewer.o floor.o pedestrian.o street.o \
#	ground.o simul.o param.o list.o fenster.o szfenster.o gui.o hash.o \
#	movie.o plot.o ${LIBZUS}
 
netfenster.o: netfenster.cc network.hh fenster.hh netfenster.hh \
	param.hh list.hh
	${CPLUS} -c -DMOTIF \
	${OPT} -I/usr/include/MIT netfenster.cc

netplot.o: netplot.cc netplot.hh param.hh
	${CPLUS} -c -g netplot.cc

network.o: network.cc network.hh list.hh vector.hh random.hh graphic.hh
	 ${CPLUS} -g -c	-I/usr/include/MIT network.cc	

Xnetwork.o: network.cc network.hh list.hh vector.hh graphic.hh
	${CPLUS} -o Xnetwork.o -g -c -I/usr/include/MIT -DMOTIF network.cc

netz.o: netz.cc network.hh list.hh fenster.hh netfenster.hh param.hh
	${CPLUS} -g -c	-I/usr/include/MIT -DMOTIF -DNETWORK \
	netz.cc		

unetz.o: unetz.cc network.hh list.hh netplot.hh param.hh random.hh
	${CPLUS} -g -c	-I/usr/include/MIT -DUNIRAS unetz.cc
	
rnetz.o: rnetz.cc network.hh list.hh netplot.hh param.hh random.hh
	${CPLUS} -g -c	-I/usr/include/MIT -DUNIRAS rnetz.cc
	
netz: netz.o Xnetwork.o list.o netfenster.o fenster.o param.o
	link netz.o Xnetwork.o list.o netfenster.o fenster.o param.o \
	${LIBZUS}

unetz: unetz.o list.o param.o network.o random.o netplot.o
	link  unetz.o list.o param.o network.o random.o netplot.o \
	 ${LIBZUS}	

rnetz: rnetz.o list.o param.o network.o random.o netplot.o
	link  rnetz.o list.o param.o network.o random.o netplot.o \
	 ${LIBZUS}	

probier.o: probier.cc param.hh
	${CPLUS} -g -c -I/usr/include probier.cc

probier: probier.o param.o 
	link  probier.o param.o ${LIBOPT}


evolution.o:	evolution.cc evolution.hh 
	$(CPLUS) -g -c evolution.cc

evolution_hp.o:	evolution.cc evolution.hh 
	$(CPLUS) -o evolution_hp.o -g -c evolution.cc

probierevol: probierevol.cc evolution.o 
	$(CPLUS) -o probierevol -g probierevol.cc evolution.o $(LIBOPT)
