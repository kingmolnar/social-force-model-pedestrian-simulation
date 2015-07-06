#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream.h>
#include <sys/wait.h>
#include <fcntl.h>


main (int argc, char** argv, char** envp) {
  pid_t pid;
  union wait *status;  
  int fildes[2]; // 0 read, 1 write
  pipe(fildes);
  
  pid = fork();
  if (pid==0) {
    // sub-process
    cout << "child\n";
    close(0); // stdin
    dup(fildes[0]);
    close(fildes[1]);
    execve("/usr/new/gnuplot/gnuplot", argv, envp);
  }
  else {
    close(fildes[0]);
    FILE *command = fdopen(fildes[1], "w");
    fprintf(command, "plot [x=-1:1] [-1:1] sin(x)\n pause 20\n quit\n");
    wait(status);
  }
  cout << "das wars " << pid << endl;
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

