#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
void pinfo(char *pid)
{
    char exec_path[2000];
    char name[2000]="/proc/";
	char name2[2000]="/proc/";
    char line[2000];
	strcat(name2,pid);
    strcat(name,pid);
    strcat(name,"/exe");
	strcat(name2,"/status");
    int fd1=open(name,O_RDONLY);
    /*if(fd1==-1)
    {
		perror("Error");
		return;
        //printf("Process doesnt exist\n");
    }*/
	
	FILE *fd2=fopen(name2,"r");
	if(!fd2)
	{
		perror("Error");
		//return;
	}
	while (fgets(line, sizeof(line), fd2) != NULL)
	{   
		if(strstr(line,"State:"))
			printf("%s", line);
		
	}
	struct rusage usage;
	readlink(name,exec_path,sizeof(exec_path));
	getrusage(RUSAGE_SELF,&usage);
	printf("PID : %s \n",pid);
	printf("Executable Path --- %s \n",exec_path);
	printf("Memory Usage --- %ld {virtual memory} \n",usage.ru_maxrss);
//	close(fd1);
	return;

}