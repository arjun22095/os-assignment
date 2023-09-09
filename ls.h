#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include<string.h>
#include<pwd.h>
#include <dirent.h>
#include<locale.h>
#include<grp.h>
#include<time.h>
char org_path_2[1000];
int isFileExistsStats(const char *path)
{
    struct stat stats;

    stat(path, &stats);

    // Check for file existence
    if ((stats.st_mode & F_OK) || (S_ISDIR(stats.st_mode))) //checking first for file then for directory
        return 1;

    return 0;
}
void get_org_2()
{
    getcwd(org_path_2,1000);
}
void ls1(char path[]) { 
    DIR * dir; struct dirent * file; 
    dir = opendir(path); 
    while(file=readdir(dir)) { 
        if(file->d_name[0]!='.')  { 
            printf("%s\n", file->d_name); 
        } 

    } 
    printf("\n");

    free(file); 
    free(dir); 
} 
void ls_a(char path[]) { 
    DIR * dir; struct dirent * file; 
    dir = opendir(path); 
    while(file=readdir(dir)) { 
        { 
            printf("%s\n ", file->d_name); 
        } 
    } 
    printf("\n");
    free(file); 
    free(dir); 
} 
void print_perms(mode_t st) { 
    char perms[11]; 
    if(st && S_ISREG(st)) perms[0]='-'; 
    else if(st && S_ISDIR(st)) perms[0]='d'; 
    else if(st && S_ISFIFO(st)) perms[0]='|'; 
    else if(st && S_ISSOCK(st)) perms[0]='s'; 
    else if(st && S_ISCHR(st)) perms[0]='c'; 
    else if(st && S_ISBLK(st)) perms[0]='b'; 
    else perms[0]='l';  // S_ISLNK 
    perms[1] = (st && S_IRUSR) ? 'r':'-'; 
    perms[2] = (st && S_IWUSR) ? 'w':'-'; 
    perms[3] = (st && S_IXUSR) ? 'x':'-'; 
    perms[4] = (st && S_IRGRP) ? 'r':'-'; 
    perms[5] = (st && S_IWGRP) ? 'w':'-'; 
    perms[6] = (st && S_IXGRP) ? 'x':'-'; 
    perms[7] = (st && S_IROTH) ? 'r':'-'; 
    perms[8] = (st && S_IWOTH) ? 'w':'-'; 
    perms[9] = (st && S_IXOTH) ? 'x':'-'; 
    printf("%s", perms); 
} 
void ls_l(char path[])
{
    DIR *dir;
    struct dirent *file;
    struct stat sbuf;
    char buf[512];
    dir = opendir(path);
    while(file=readdir(dir)) 
    {   
        sprintf(buf, "%s", file->d_name);
        if(buf[0]=='.') continue;
        stat(buf, &sbuf);
        struct passwd *tf = getpwuid(sbuf.st_uid);  //get user id
        struct group *grp = getgrgid(sbuf.st_gid); //get group id
       
        print_perms(sbuf.st_mode);
        printf(" %ld ", sbuf.st_nlink);
        printf(" %s ", tf->pw_name);        
        printf(" %s ", grp->gr_name);
        printf(" %.19s ", ctime(&sbuf.st_mtime));
        printf(" %lu ",sbuf.st_size);
        printf(" %s ", file->d_name);
        printf("\n");

    }
    closedir(dir);

}
void ls_la(char path[])
{
     DIR *dir;
    struct dirent *file;
    struct stat sbuf;
    char buf[512];
    dir = opendir(path);
    while(file=readdir(dir)) 
    {   
        sprintf(buf, "%s", file->d_name);
        //if(strcmp(buf,".")==0 || strcmp(buf,"..")==0) continue;
        stat(buf, &sbuf);
        struct passwd *tf = getpwuid(sbuf.st_uid);
        struct group *grp = getgrgid(sbuf.st_gid);
       
        print_perms(sbuf.st_mode);
        printf(" %ld ", sbuf.st_nlink);
        printf(" %s ", tf->pw_name);        
        printf(" %s ", grp->gr_name);
        printf(" %.19s ", ctime(&sbuf.st_mtime));
        printf(" %lu ",sbuf.st_size);
        printf(" %s ", file->d_name);
        printf("\n");

    }
    closedir(dir);
}
int ls(char ** args)
{

    int l_flag=0;
    int a_flag=0;
    int i=1,j=0;
    
    char arr[128][128];
    
        while(args[i]!=NULL)
        {
            if (strcmp(args[i],"-l")==0)
            {
                l_flag=1;
            }
            else if(strcmp(args[i],"-a")==0)
            {
                a_flag=1;
            }
            else if(strcmp(args[i],"-la")==0)
            {
                l_flag=1;
                a_flag=1;
            }
            else if(strcmp(args[i],"-al")==0)
            {
                l_flag=1;
                a_flag=1;                
            }
            
            else
            {
                strcpy(arr[j++],args[i]);
            }
            i++;
        }

    char name[128];
    getcwd(name,128);
    if(args[1]==NULL)
    {
       
        ls1(name);
    }
    else if (j==0)
    {
        if(a_flag && l_flag)
        {
            ls_la(name);
        }
        else if(a_flag)
        {
            ls_a(name);
        }
        else if(l_flag)
        {
            ls_l(name);
        }

    }
    else
    {
        for(int k=0;k<j;k++)
        {
        
            strcat(name,"/");
            strcat(name,arr[k]);
            if(l_flag ==1 && a_flag==1)
            {
                if(!isFileExistsStats(arr[k]))
                {
                    perror("");
                }

                else if(strcmp(arr[k],"~")==0)
                {
                    printf("%s\n",arr[k]);
                    ls_la(org_path_2);
                }
                else
                {
                    printf("%s\n",arr[k]);
                    ls_la(arr[k]);

                }
            
            
            }
            else if(l_flag==1)
            {
                if(!isFileExistsStats(arr[k]))
                {
                    perror("");
                }
                else if(strcmp(arr[k],"~")==0)
                {
                    printf("%s\n",arr[k]);
                    ls_l(org_path_2);
                }
                else
                {
                    printf("%s\n",arr[k]);
                    ls_l(arr[k]);
                }
            }
            else if(a_flag==1)
            {
                if(!isFileExistsStats(arr[k]))
                {
                    perror("");
                }
                else if(strcmp(arr[k],"~")==0)
                {
                    printf("%s\n",arr[k]);
                    ls_a(org_path_2);
                }
                else
                {
                    printf("%s\n",arr[k]);
                    ls_a(arr[k]);
                }
            }
            else
            {
                if(!isFileExistsStats(arr[k]))
                {
                    perror("");
                }
                else if(strcmp(arr[k],"~")==0)
                {
                    printf("%s\n",arr[k]);
                    ls1(org_path_2);
                }
                else
                { 
                    printf("%s\n",arr[k]);
                //printf("gel");
                    ls1(arr[k]);
                }
            }
        }

    }
    return 1;
}