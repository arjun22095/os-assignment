// file to store all the all the stuff
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
char shell_first_name[1000]="<";
char shell_mid_name[1000];
char shell_name2[100]=">";
// last directory opened in the folder;
char org_directory[2000];
char curr_directory[2000];
int len_org;
