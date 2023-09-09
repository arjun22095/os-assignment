char *readline()
{
    char* line=NULL;
    ssize_t buffer=0;
    int is_line=getline(&line,&buffer,stdin);
    if(is_line==-1)
    {
        if(feof(stdin))
        {
            exit(0);
        }
        else{
            perror("Error Occured");
            exit(0);
        }
    }
    return line;

}
char **getcommands(char * line)
{
    int idx=0;
    char **token_list=malloc(1000*sizeof(char));
    char * token;
    token=strtok(line, ";");
    while(token!=NULL)
    {
        token_list[idx]=token;
        idx++;
        token = strtok(NULL,";");
    }
    token_list[idx]=NULL;
    return token_list;

}
char **get_tokens(char * line)
{
    int idx=0;
    char **token_list=malloc(1000*sizeof(char));
    char * token;
    token=strtok(line, " \t\r\n\a");
    while(token!=NULL)
    {
        token_list[idx]=token;
        idx++;
        token = strtok(NULL," \t\r\n\a");
    }
    token_list[idx]=NULL;
    return token_list;
}

