#include "call.hh"
#include <unistd.h>
#include <iostream.h>
#include <sys/wait.h>
#include <fcntl.h>


program::program(char *n, char** argv, char** envp) {
  //  union wait *status;  
  name = n;
  pipe(fildes);
  pid = fork();
  if (pid==0) {
    // sub-process
    close(0); // stdin
    dup(fildes[0]);
    close(fildes[1]);
    execve("/usr/new/gnuplot/gnuplot", argv, envp);
  }
  else {
    close(fildes[0]);
    FILE *fp = fdopen(fildes[1], "w");
  }
} 

program::~program() {
  close(fildes[0]);
  close(fildes[1]);
  fclose(fp);
}
  
/* 
   #include <sys/types.h>
   pid_t pid;
   int pipefd[2];
   
   //  Assumes file descriptor 0 and 1 are open  
   pipe (pipefd);
   
   if ((pid = fork()) == (pid_t)0) {
   close(1);      // close stdout
   dup (pipefd[1]);
   close (pipefd[0]);
   execlp ("ls", "ls", (char *)0);
   }
   else if (pid > (pid_t)0) {
   close(0); // close stdin  
   dup (pipefd[0]);
   close (pipefd[1]);
   execlp ("sort", "sort", (char *)0);
   }
   */

