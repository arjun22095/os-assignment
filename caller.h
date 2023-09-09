#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
int min(int a,int b)
{
    if(a<b) return a;
    return b;
}
int caller(char ** args)
{
    
    if(args[0]==NULL)
    {
        // empty command nothing given
        //printf("no arguments given");
        return 1;
    }
   // printf("%s\n",args[0]);
    if (strcmp(args[0],"cd")==0)
    {  
       return (cd_func(args));
    }
    else if (strcmp(args[0],"exit_s")==0)
    {
       return (exit_shell(args));
    }
    else if(strcmp(args[0],"ls")==0)
    {
       return ls(args);
    }
    else if (strcmp(args[0],"pwd")==0)
    {
         return(get_pwd());
    }
    else if(strcmp(args[0],"echo")==0)
    {
        return (echo_str(args));
    }
    else if(strcmp(args[0],"history")==0)
    {
        if(args[1]==NULL)
            history(20);
        else 
        {
            history(min(20,atoi(args[1])));
        }
  
        return 1;
    }
    else if(strcmp(args[0],"pinfo")==0)
    {
        if(args[1]==NULL)
        {
          
          int pd=getpid();
            char temp[10];
            sprintf(temp,"%d",pd);
            pinfo(temp);
        }
        else
        {
            pinfo(args[1]);
        }
        return 1;
    }
    else if(strcmp(args[0],"nightwatch")==0)
    {
        if(strcmp(args[1],"-n")==0)
        {
            if(strcmp(args[3],"interrupt")==0)
            {
                interrupt1(atoi(args[2]));
            }
            if(strcmp(args[3],"newborn")==0)
            {
                newborn(atoi(args[2]));
            }
            return 1;
        }
        else 
        {
            printf("Invalid");
            return 1;
        }
    }
    return start_process(args);
    
}