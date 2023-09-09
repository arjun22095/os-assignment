#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h>  
#include <sys/wait.h>
#include<sys/types.h>
#include<sys/resource.h>
#include <sys/utsname.h>
#include<errno.h>
int bg_process[10000];
int idx=0;
void signalHandler(int signal)
{
    int status;
    pid_t pid;
    pid=waitpid(-1,&status,WNOHANG | WUNTRACED);        
    while(pid!=-1)
    {
        if(!pid) break;       
        for(int i=0; i<idx; i++)
        {
            if(pid == bg_process[i])
            {
                if(WIFEXITED(status))
                    printf("Process : %d finished properly\n", pid);
                else
                    printf("Process : %d exitted abruptly\n", pid);

            }
        }
        pid=waitpid(-1,&status,WNOHANG | WUNTRACED);
    }
}
  
int start_process(char ** args)
{
    pid_t pid;
    int status;
    int k=0;
    while(args[k]!=NULL)
    {
        k++;
    }
    if(k==0)
    {
        printf("Insufficient Arguments");
    }       
    signal(SIGCHLD,signalHandler);
    pid=fork();
    if(pid==0)
    {
        // child process to be executed first
        if(k>=1 && strcmp(args[k-1],"&")==0)
        {
            args[k-1]=NULL;
           // bg=1;
            setpgid(getpid(),0);
            printf("%d\n",getpid());
            if(execvp(args[0],args)==-1) 
            {
                perror("Commans not found");
            }
        }
        else
        {
            if(execvp(args[0],args)==-1) 
            {
                perror("Command Not Found");
            }
        }
        exit(0);
    }
    else if (pid<0)
    {
        perror("Error");
        return 0;
    }
    else
    {

        if(strcmp(args[k-1],"&")!=0)
        {
            waitpid(pid,&status,0);
        }
        else
        {
            bg_process[idx++]=pid;
            printf("[%d]\n",idx);
        }

     
    }
   return 1;
}