#include<stdio.h> 
#include<fcntl.h> 
#include<errno.h> 
char path[2000];
char last[2000]="@@";
int hist_count=0;
void get_org_3()
{
    getcwd(path,sizeof(path));
    strcat(path,"/hist.txt");
    
}
void add_to_history(char *line)
{
    hist_count++;
    FILE *fd;
    fd=fopen(path,"a");
    if(strcmp(last,line)==0) return;
    strcpy(last,line);
    if(fd==NULL)
    {
        perror("");
    }
    else 
    {
        fprintf(fd, "%s",line);
        //fclose(fd);
    }
    fclose(fd);
    fd=fopen(path,"r");
    if(fd==NULL)
    {
        perror("");
        return ;
    }
    int idx=0;
    char arr[20][1024];
    char line1[1024];
    while(fgets(line1,sizeof(line1),fd))
    {
      //  printf("%s",line1);
        strcpy(arr[idx],line1);
        idx++;
    }
//printf("%d",idx);
    if(idx<20)
    {
        fclose(fd);
        return ;

    }
    fclose(fd);
    fd=fopen(path,"w+");
    if(fd==NULL)
    {
        perror("");
        return ;
    }
    for(int i=idx-20;i<idx;i++)
    {

      //  printf("%d %s\n",i,arr[i]);
        fputs(arr[i],fd);
    }
    fclose(fd);
    
}
void history(int num)
{
    char line[1000];
    FILE *fd=fopen(path,"r");
    int x=0;
    hist_count=0;
    while(fgets(line,sizeof(line),fd))
    {
        hist_count++;
    }
    fclose(fd);
    FILE *fd1=fopen(path,"r");
    //printf("%d",hist_count);
    while(fgets(line, sizeof(line),fd1))
    {
    
        if(x>=(hist_count-num))
           printf("%s", line);
        x++;
    }
    return;
}