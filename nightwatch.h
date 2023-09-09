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
#include<termios.h>
#include<time.h>
void delay(int number_of_seconds) 
{ 
    // Converting time into milli_seconds 
    int milli_seconds = 1000 * number_of_seconds; 
  
    // Storing start time 
    clock_t start_time = clock(); 
  
    // looping till required time is not achieved 
    while (clock() < start_time + milli_seconds) 
        ; 
}

void newborn(int timer)
{
    int status;
    int pid=fork();
    int flag=0;
    if(pid==0)
    {
        setpgid(getpid(),0);
        while(1)
        {
        //   printf("%d",timer);
            delay(timer*100);
            char name[1000]="/proc/loadavg";
            FILE* fd=fopen(name,"r");
            char line[1000];
            fgets(line,sizeof(line),fd);
            int idx=0;
        //  printf("%s",line);
            for(int i=0;i<strlen(line);i++)
            {
                if(line[i]==' ')
                {
                    idx=i;
                }
            }
            for(int i=idx+1;i<strlen(line);i++)
            {
                printf("%c",line[i]);
            }
            fclose(fd);
        }
    }
    else if(pid<0)
    {
        perror("");
    }
    else
    {
        while(1)
        {
            char x=getchar();
            if(x=='q')
            {
                kill(pid,SIGINT);
                return ;
            }

        }
    }
  
        
}

void interrupt1(int timer)
{
    int status;
    int pid=fork();
    int flag=0;
    if (pid==0)
    {
        setpgid(getpid(),0);
        while(1)
        {
            delay(timer*100);
            FILE *fd=fopen("/proc/interrupts","r");
            if(fd==NULL)
            {
                perror("");
            }
            char line[100];
            fgets(line,sizeof(line),fd);
            printf("%s\n",line);
            fgets(line,sizeof(line),fd);
            fgets(line,sizeof(line),fd);
            printf("%s\n",line);
            fclose(fd);
            //char ch=getchar();
        }
    }
    else if(pid<0)
    {
        perror("");
    }
    else 
    {
        while(1)
        {
            char ch=getchar();
            if(ch=='q')
                kill(pid,SIGINT);
            return ;
        }
        
    }
    return ; 
}
