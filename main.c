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
#include "cd_pwd.h"
#include "nightwatch.h"
#include "exit_shell.h"
#include "input.h"
#include "history.h"
#include"echo.h"
#include"ls.h"
#include"proc.h"
#include "process.h"
#include "caller.h"
void get_name()
{
    char temp[1000];
    strcpy(temp,shell_first_name);
    strcat(temp,shell_mid_name);
    strcat(temp,shell_name2);
    printf("%s",temp);
}
void prompt()
{
    get_original(); // get path of original file
    get_org_2(); // copy of original file path
    get_org_3(); // another copy of the original file path:)
    printf("*****************************\n\n\n");
    printf("WELCOME TO MY SHELL\n\n\n");
    printf("****************************\n\n");
  // get hostname and user name
    char host[1024];
    host[1023]='\0';
    gethostname(host,1023);
    char *username;
    username = getpwuid(getuid())->pw_name;
    strcat(shell_first_name,username);
    strcat(shell_first_name,"@");
    strcat(shell_first_name,host);
    strcat(shell_first_name,":~");
    
}
void runing_loop()
{
    int status;
    char * line;
    char **tokens;
    prompt(); 

    while(1)
    {
        get_name();
       // printf("%s",shell_name);
        fflush(stdout);
        line=readline();
        char **commands=getcommands(line);
        int j=0;
        while(commands[j]!=NULL)
        {
           // get_name();
            add_to_history(commands[j]);
            char **tokens=get_tokens(commands[j]);
            status=caller(tokens);
            free(tokens);
            if (status==0) 
            {
                printf("EXITTING SHELL\n");
                printf("BBYE!!!\n");
                return ;
                break;
            }
                j++;
        }

        free(commands);
        free(line);
    }
}
int main(int argc, char *argv[])
{
    runing_loop();
    return 0;
}