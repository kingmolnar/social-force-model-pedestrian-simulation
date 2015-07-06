#define NTEST
extern "C" {
#include <string.h>
#include <assert.h>
int strcasecmp(const char*, const char*);
}
#include "test.h"
#include <iostream.h>
#include "textfile.hh"

TextFile::TextFile(char *n, char *m) {
  name = new char[strlen(n)+1];
  strcpy(name, n);
  mod= new char[strlen(m)+1];
  strcpy(mod, m);
  stream = fopen(name, mod);
  if (stream) flag = 1;
  else flag = 0;
  li = 0;
  co = 0;
  buffer = new char[buffersize];
  bufferind = 0;
}

TextFile::~TextFile() {
  if (stream) fclose(stream);
  if (name) delete name;
  if (buffer) delete buffer;
};


void TextFile::reopen() {
  if (stream) fclose(stream);
  li = co = 0;
  if (name && mod) {
    stream = fopen(name, mod);
    if (stream) flag = 1;
    else flag = 0;
  }
}

static const int TOKENLENGTH = 512;

void TextFile::token(char *tk) {
  int n = 0;
//  static FILE *altstr = NULL;
  static int c = 0; // fgetc returns int
  char *t = tk;
  enum { kommentar, weiter, zeichenkette} st = weiter;
  // in der Routine fehlt noch die Behandlung von Zeichenketten!
  
  while (n<TOKENLENGTH) {
    if (!c) {
      c = fgetc(stream);
      buffer[bufferind] = c;
      bufferind = (bufferind+1)%buffersize;
      switch (c) {
      case '\n': case EOF:
	li++;
	co = 0;
	break;
      default:
	co++;
	break;
      }
    }
    switch (st) {
    case weiter:
      switch (c) {
      case ' ': case '\t': case '\n':
	if (n==0) c = 0;
	else { // token abschliessen und zeichen wegwerfen
	  c = *t = 0;
	  return;
	}
	break;
      case '#': // Kommentar einleiten bis Zeilenende
	c = 0;
	st = kommentar;
	break;
      case '"': // Zeichenkette einleiten bis "
	*t++ = '`'; n++;
	c = 0;
	st = zeichenkette;
	break;
      case EOF: case ';': /* case ',': */
      case '=': case '[': case ']':
      case '{': case '}': case '<': case '>': case '^': case ':':
	// einzelnes Zeichen
	if (n==0) {
	  *t++ = c;
	  c = *t = 0;
	  return;
	}
	else { // token abschliessen und zeichen behalten
	  *t = 0;
	  return;
	}
      default:
	*t++ = c; n++;
	st = weiter;
	c = 0;
	break;
      } // end switch (c)
      break;
    case kommentar:
      switch (c) {
      case '\n': case EOF: 
	if (n==0) {
	  if (c=='\n') c = 0; // nur eine kommentarzeile, weiterlesen
	  st = weiter;
	}
	else { // token abschliessen und zeichen wegwerfen
	  c = *t = 0;
	  return;
	} 
      default:
	c = 0;
	break;
      }
      break;
    case zeichenkette:
      switch (c) {
      case EOF: // fehler
	c = *t = 0;
	return;
      case '"':
	c = *t = 0;
	return;
      default:
	*t++ = c; n++;
	st = zeichenkette;
	c = 0;
	break;
      }
      break;
    } // end switch (st)
  } // end while
  *t = 0; // die restlichen zeichen koennten an dieser Stelle abgeschnitten
  // werden
  return;
}

char* TextFile::lastline(int n) {
  if (n<=0) n = 1;
  int b = bufferind;
// char *p = buffer+bufferind; 
  if (buffer[bufferind]=='\n') {
    buffer[(b+1)%buffersize] = 0;
    b = (b-1)%buffersize;
  }
  else {
    buffer[(b+1)%buffersize] = '\n';
    buffer[(b+2)%buffersize] = 0;
  }
  b += buffersize;
  char c;
  int i;
  for (i=0; i<buffersize; ++i) {
    c = buffer[(b-i)%buffersize];
    if (c=='\n') n--;
    if (n==0 || c=='\0') break;
  } 
  return buffer+((b+1-i)%buffersize);
}
      

// parse(TextFile *fp) {
//   enum stat {S, Z, E1, E2, E3, E4, D1, D2, D3, D4, L, M1, M2, I1, F} st = S;
//   char token[TOKENLENGTH+1];
//   char identifier[TOKENLENGTH+1];
//   char fehler[80];
//   SimulObjectType tp;
//   SimulObjectNode *pn, *par;
//   SimulObject *cl;
//   ParameterValue *p, pv;
//   double *values = new double[32];
//   Vector *vectors = new Vector[32];
//   int nvalues = 0, nvectors = 0;
//   int multi, nident, nmulti, npar;
//   const int True = 1, False = 0;
  
//   while(st!=Z && st!=F) {
//     fp->token(token);
//     test(printf("status: %d ->%s<-\n", st, token));
//     switch (st) {
//     case S: // Start
//       multi = 0; nident = nmulti = 0;
//       *identifier = '\0';
//       par = this; npar = 0;
//       tp = _UNDEF;
//       //if (*token==(char)EOF) { // 
//       if (fp->eof()) {
// 	st = Z; 
//       }
//       else {
//       }
//       break;
//     default:
//       break;
//     }
//   }
// }
