#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include <fcntl.h>  
#include <sys/wait.h>
#include<sys/types.h>
#include <sys/utsname.h>
#include<errno.h>
#include "global_var.h"
// function to get the original directory from where shell started
void get_original(void)
{
    if(getcwd(org_directory,sizeof(org_directory))!=NULL)
    {
        //printf("%s",org_directory);
      len_org=strlen(org_directory);
    }
    else{
        perror("Error in getting cwd");

    }
}

// funciton to get directory at any time in the program
void get_curr(void)
{
    if(getcwd(curr_directory,sizeof(curr_directory))!=NULL)
    {
      //  printf("%s",curr_directory);
    }
    else{
        perror("Error in getting cwd");

    }
}
int cd_func(char **args)
{
   // printf("changing directory\n");
    if(args[1]==NULL)
    {
        chdir("~");
        chdir(org_directory);
        strcpy(curr_directory,org_directory);
        strcpy(shell_mid_name,curr_directory+len_org);
        return 1;
    }
    if(args[1][0]=='/')
    {
        chdir("/");
        get_curr();
        if(strlen(curr_directory)<len_org)
        {
            strcpy(shell_mid_name,curr_directory);
        }
    }
    else
    {
        if (strcmp(args[1],".")==0)
        {
            get_curr();
            printf("%s\n",curr_directory+len_org);
        }
        else if(strcmp(args[1],"~")==0)
        {
            chdir("~");
            chdir(org_directory);
            get_curr();
            strcpy(shell_mid_name,curr_directory+len_org);


        }
        else
        {
            int xx=chdir(args[1]);
            if(xx==-1)
            {
                perror("Error occured");
                return 1;
            }
            else
            {

                char temp[1000];
                get_curr();
                strcpy(shell_mid_name,curr_directory+len_org);
            }
        }
    
        
    }
    if(strstr(curr_directory,org_directory)==NULL)
    {
        get_curr();
        strcpy(shell_mid_name,curr_directory);
        //getcwd(shell_mid_name,sizeof(shell_mid_name));
    }
    return 1;
}
int get_pwd(void)
{
    get_curr();
    if(strcmp(curr_directory,org_directory)==0)
    {
        printf("/\n");
    }
    else if(strstr(curr_directory,org_directory)==NULL)
    {
        get_curr();
        printf("%s\n",curr_directory);
    }
    else
    printf("%s\n",curr_directory+len_org);
    return 1;

}