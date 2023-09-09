int echo_str(char **args)
{
    int idx=1;
    while(args[idx]!=NULL)
    {
        //if(args[idx]==NULL) continue;
        printf("%s ",args[idx]);
        idx++;
    }
    printf("\n");
    return 1;
}