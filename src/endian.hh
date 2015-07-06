/*
 *	endian.hh
 *	Routines for little endian -- big endian conversion
 *	and platform independent little endian and big endian
 *	output routines.
 *
 */

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <iostream.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "hardware.h"
#define _int_siz sizeof(int)   

unpack_netbor_2_4(int* src, int n);
unpack_netbor_2_4(unsigned int* src, int n);
pack_netbor_4_2(int* src, int n);
inline
pack_netbor_4_2(unsigned int* src, int n=1)
	{ pack_netbor_4_2((int*) src, n); }

int read_netbor2(int fd, int* i, int n=1);
int read_netbor2(int fd, unsigned int* ui, int n=1);

int read_netbor4(int fd, int* i, int n=1);
int read_netbor4(int fd, unsigned int* ui, int n=1);
int read_netbor4(int fd, float* f, int n=1);
int read_netbor4(int fd, double* d, int n=1);

int write_netbor2(int fd, const int* i, int n=1);
inline
int write_netbor2(int fd, const unsigned int* ui, int n=1)
	{write_netbor2(fd, (int*) ui, n);}

int write_netbor4(int fd, const int* i, int n=1);
inline
int write_netbor4(int fd, const unsigned int* ui, int n=1)
	{write_netbor4(fd, (int*) ui, n);}
int write_netbor4(int fd, const float* f, int n=1);
int write_netbor4(int fd, const double* d, int n=1);


// the write procedures, call by value
inline
int write_netbor2(int fd, const int &i)
	{write_netbor2(fd, &i, 1);}
inline
int write_netbor2(int fd, const unsigned int &ui)
	{write_netbor2(fd, (int*) &ui, 1);}

inline
int write_netbor4(int fd, int i)
	{write_netbor4(fd, &i, 1);}
inline
int write_netbor4(int fd, unsigned int ui)
	{write_netbor4(fd, (int*) &ui, 1);}
inline
int write_netbor4(int fd, float f)
	{write_netbor4(fd, &f, 1);}
inline
int write_netbor4(int fd, double d)
	{write_netbor4(fd, &d, 1);}
#endif
