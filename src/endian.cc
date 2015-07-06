/*
 *	endian.cc
 *	Routines for endian conversion
 */

#include "endian.hh"

/*
 * All routines in this file base on big endian byte order.
 * If necessary, byte order conversion is performed by
 * the htonl/htons and ntohl/ntohs tcp/ip functions (macros)
 */

unpack_netbor_2_4(int* src, int n)
{
  char *buf;
  buf=(char*) src;

  register char fill;

  for (register int i=n-1; i>=0; i--)
    {
      buf[(i<<2)+3]	= buf[(i<<1)+1];
      fill=((buf[(i<<2)+2]=buf[(i<<1)]) & 0x80)?-1:0;
      buf[(i<<2)+1]	= fill;
      buf[(i<<2)]	= fill;
    }  
}

unpack_netbor_2_4(unsigned int* src, int n)
{
  char *buf;
  buf=(char*) src;

  for (register int i=n-1; i>=0; i--)
    {
      buf[(i<<2)+3]	= buf[(i<<1)+1];
      buf[(i<<2)+2]	= buf[(i<<1)];
      buf[(i<<2)+1]	= (char)0;
      buf[(i<<2)]	= (char)0;
    }  
}

pack_netbor_4_2(int* src, int n)
{
  char fill, *buf;
  buf=(char*) src;

  for (register int i=0; i<n; i++) {
    buf[(i<<1)]		= buf [(i<<2)+2];
    buf[(i<<1)+1]	= buf [(i<<2)+3];
  }
}

int read_netbor2(int fd, int* i, int n)
{
  int rc;
  // read 2*n bytes from fd into the buffer i:
  // network byte order (big endian)

  if ( -1 == ( rc = read(fd, (char *) i, n<<1) ) ) return -1;

  rc >>=1;

  // unpack the array of 2 byte ints to full size ints:
  unpack_netbor_2_4(i,rc);

  // convert from network byte order to host byte order:
  for (register int j=0; j<rc; ++j) i[j]=ntohl(i[j]);

  return rc;
}

int read_netbor2(int fd, unsigned int* ui, int n)
{
  int rc;
  // read 2*n bytes from fd into the buffer i:
  // network byte order (big endian)

  if ( -1 == ( rc = read(fd, (char *) ui, n<<1) ) ) return -1;

  rc >>=1;

  // unpack the array of 2 byte ints to full size ints:
  unpack_netbor_2_4(ui,rc);

  // convert from network byte order to host byte order:
  for (register int j=0; j<rc; ++j) ui[j]=ntohl(ui[j]);

  return rc;
}

int read_netbor4(int fd, int* i, int n)
{
  int rc;
  // read 4*n bytes from fd into the buffer i:
  // network byte order (big endian)

  if ( -1 == ( rc = read(fd, (char *) i, n<<2) ) ) return -1;

  rc >>=2;

  // unpack the array of 4 byte ints to full size ints:
  // unpack_netbor_4_4(i,rc);

  // convert from network byte order to host byte order:
  for (register int j=0; j<rc; ++j) i[j]=ntohl(i[j]);

  return rc;
}

int read_netbor4(int fd, unsigned int* ui, int n)
{
  int rc;
  // read 4*n bytes from fd into the buffer i:
  // network byte order (big endian)

  if ( -1 == ( rc = read(fd, (char *) ui, n<<2) ) ) return -1;

  rc >>=2;

  // unpack the array of 2 byte ints to full size ints:
  // unpack_netbor_4_4(ui,rc);

  // convert from network byte order to host byte order:
  for (register int j=0; j<rc; ++j) ui[j]=ntohl(ui[j]);

  return rc;
}

int read_netbor4(int fd, float* f, int n)
{
  // make i and ui identical to pointer d but of types int* and unsigned int*
  int* i= (int *) f;

  // now read in the doubles as if they were integers
  int rc = read_netbor4(fd,i,n);

  // now perform conversions to reflect your host's float representation
  // if (rc != -1) float_iee_host(f,rc);

  return rc;
}

int read_netbor4(int fd, double* d, int n)
{
  int* i= (int *) d;
  float* f= (float *) d;
  int rc = read_netbor4(fd,i,n);
  if (rc != -1) {
    // float_iee_host(f,rc);
    for (register int j=rc-1; j>=0; --j) d[j]=f[j];
  }
  return rc;
}


int write_netbor2(int fd, const int* i, int n)
{
  int rc;
  int* buf;

  // create a buffer to hold the data since we don't know if we
  // may destroy it. Return -1 if we don't get the memory.

  if ( ! (buf = new int[n])) return -1;

  // now convert them to big endian.
  for (register int j=0; j<n; ++j) buf[j]=htonl (i[j]);

  // pack them
  pack_netbor_4_2(buf,n);

  // write 2*n bytes from buf into file fd:
  rc = write(fd, (char *) buf, n<<1);

  delete [] buf;

  return ((rc == -1)? -1 : rc>>1);
}

int write_netbor4(int fd, const int* i, int n)
{
  int rc;
  int* buf;

  if ( ! (buf = new int[n])) return -1;
  for (register int j=0; j<n; ++j) buf[j]=htonl(i[j]);
  rc = write(fd, (char *) buf, n<<2);
  delete [] buf;
  return ((rc == -1)? -1 : rc>>2);
}

int write_netbor4(int fd, const float* f, int n)
{
  int rc;
  int* i;
  float* buf;

  if ( ! (buf = new float [n])) return -1;
  i=(int*)buf;

  memcpy((void*) buf, (void*) f, n<<2);
  //host_to_iee(float,n);
  for (register int j=0; j<n; ++j) i[j] = htonl (i[j]);

  rc = write(fd, (char *) i, n<<2);

  delete [] buf;

  return ((rc == -1)? -1 : rc>>2);
}

int write_netbor4(int fd, const double* d, int n)
{
  int rc;
  int* i;
  float* buf;

  if ( ! (buf = new float [n])) return -1;
  i=(int*)buf;

  for (register int j=0; j<n; ++j) {
    buf[j]=d[j];
    //host_to_iee(buf,1);
    i[j] = htonl (i[j]);
  }

  rc = write(fd, (char *) i, n<<2);

  delete [] buf;

  return ((rc == -1)? -1 : rc>>2);
}
