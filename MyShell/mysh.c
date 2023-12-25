#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>


#define MAXBUFFER 512
int numOfTokens=1;      //How many Tokens exist
int BytesRead;
int numOfWildcards=1;
int errorcheck=0;        //If there was a inappropriate command, it will fail.   0 is no error. 1 is error
int batchExit = 0;



// ____________________________________________________________________________________________________________
//|                                              STRUCT INFO                                                   |
//|____________________________________________________________________________________________________________|



struct commandInfo{
    char* path;                 //The path to the file
    char* command;             //The first token will always be the command to run
    char** arguments;           //List of all arguments stored in each element of a char array, the last value of the array will be set to NULL
    int numOfArguments;         //Number of arguments stored inside the array
    char* inputRedirection;     //File that will be used for input redirection. There can't be multiple IO redirections
    char* outputRedirection;    //File that will be used for output redirection. There can't be multiple IO redirections.       NO OUTPUT REDIRECTION IF THERES PIPE
    int isPipe;                 //Boolean to check if it has a pipe
    struct outputPipeCommandInfo* pipeOutput;       //Pointer in the event we have a output Pipe
};

struct outputPipeCommandInfo{
    char* path;                 //The path to the file
    char* command;             //The first token will always be the command to run
    char** arguments;           //List of all arguments stored in each element of a char array, the last value of the array will be set to NULL
    int numOfArguments;         //Number of arguments stored inside the array
    char* inputRedirection;     //File that will be used for input redirection. There can't be multiple IO redirections         NO INPUT REDIRECTION IF THERES PIPE
    char* outputRedirection;    //File that will be used for output redirection. There can't be multiple IO redirections.
    struct commandInfo* input;       //Pointer in the event we have a output Pipe
};



// ____________________________________________________________________________________________________________
//|                                           PARSING ALGORITHM                                                |
//|____________________________________________________________________________________________________________|



char** parsing(char* buffer){
    int bufferindex=0;      //This is a variable that goes through buffer and attemps to copy
    int wordsize=1;         //How long is the current token
    int readyForWord=0;     //If we have not encountered a space, it will not create a new token. 0 for false, 1 for true

    char **tokens = (char**)malloc(numOfTokens*sizeof(char*));      //Char pointer array that points to the tokens
    tokens[0] = (char*)malloc(wordsize);                            //First pointer is initialized as a wordsize of 1
    
    for(bufferindex=0; bufferindex<BytesRead-1; bufferindex++){
        if(buffer[bufferindex]==' ' && readyForWord == 0 && wordsize!= 1){      //A word just ended, prepare for new token
            readyForWord = 1;
            tokens[numOfTokens-1][wordsize-1] = '\0';
            continue;
        }

        if(buffer[bufferindex]==' ' && readyForWord == 1 && wordsize!= 1){      //If we have a repeated space
            continue;
        }

        if( ( (buffer[bufferindex]== '<') || (buffer[bufferindex]== '>') || (buffer[bufferindex]== '|') ) && (readyForWord==1) ){    //If there is a special character
            numOfTokens++;      //Increase the number of tokens because this is a new token that needs to be saved
            wordsize=2;         //We are only copying 1 character
            tokens = (char**)realloc(tokens,numOfTokens*sizeof(char*));     //Increasing the number of tokens to accomodate for the new token
            tokens[numOfTokens-1] = (char*)malloc(wordsize*sizeof(char));                //Making the new token point to an empty char
            tokens[numOfTokens-1][0] = buffer[bufferindex];        //Here we will copy that exact character
            tokens[numOfTokens-1][1] = '\0';
            readyForWord=1;     //After this point, we should be expecting to create a brand new token
            continue;
        }

        if( ( (buffer[bufferindex]== '<') || (buffer[bufferindex]== '>') || (buffer[bufferindex]== '|') ) && (readyForWord==0) ){    //If there is a special character
            tokens[numOfTokens-1][wordsize-1] = '\0';
            numOfTokens++;      //Increase the number of tokens because this is a new token that needs to be saved
            wordsize=2;         //We are only copying 1 character
            tokens = (char**)realloc(tokens,numOfTokens*sizeof(char*));     //Increasing the number of tokens to accomodate for the new token
            tokens[numOfTokens-1] = (char*)malloc(wordsize*sizeof(char));                //Making the new token point to an empty char
            tokens[numOfTokens-1][0] = buffer[bufferindex];        //Here we will copy that exact character
            tokens[numOfTokens-1][1] = '\0';
            readyForWord=1;     //After this point, we should be expecting to create a brand new token
            continue;
        }

        if( (readyForWord == 0) && (buffer[bufferindex]!=' ') ){          //if we are reading the middle of a word
            tokens[numOfTokens-1][wordsize-1] = buffer[bufferindex];        //Here we will copy that exact character
            wordsize++;
            tokens[numOfTokens-1] = (char*)realloc(tokens[numOfTokens-1],wordsize*sizeof(char));                //Making the new token point to an empty char
            continue;
        }

        if( (readyForWord == 1) && (buffer[bufferindex]!=' ') ){          //if we are reading the start of a word
            numOfTokens++;      //Increase the number of tokens because this is a new token that needs to be saved
            wordsize=1;         //We are only copying 1 character
            tokens = (char**)realloc(tokens,numOfTokens*sizeof(char*));     //Increasing the number of tokens to accomodate for the new token
            tokens[numOfTokens-1] = (char*)malloc(wordsize);                //Making the new token point to an empty char
            tokens[numOfTokens-1][wordsize-1] = buffer[bufferindex];    //Here we will copy that exact character
            wordsize++;
            tokens[numOfTokens-1] = (char*)realloc(tokens[numOfTokens-1],wordsize*sizeof(char));                //Making the new token point to an empty char
            readyForWord=0;
            continue;
        }
    }
    tokens[numOfTokens-1][wordsize-1] = '\0';       //Make sure the last word has a terminator

    return tokens;
}



// ____________________________________________________________________________________________________________
//|                                           FREE TOKEN FOR LOOP                                              |
//|____________________________________________________________________________________________________________|



void freeing(char** tokens){            //Function to clear up the array
    for(int i=0; i<numOfTokens; i++){
        free(tokens[i]);
    }
    free(tokens);
    numOfTokens=1;
}

void firststructfreeing(struct commandInfo* front ){
    free(front->path);
    free(front->command);
    for(int i=0; i<front->numOfArguments; i++){
        free(front->arguments[i]);
    }
    free(front->arguments);
    free(front->inputRedirection);
    free(front->outputRedirection);
    
}

void pipestructfreeing(struct outputPipeCommandInfo* front ){
    free(front->path);
    free(front->command);
    for(int i=0; i<front->numOfArguments; i++){
        free(front->arguments[i]);
    }
    free(front->arguments);
    free(front->inputRedirection);
    free(front->outputRedirection);
    
    free(front);
}



// ____________________________________________________________________________________________________________
//|                                          INITIALIZE STRUCT                                                 |
//|____________________________________________________________________________________________________________|



void initializeCommandStruct(struct commandInfo* front){
    front->path = NULL;
    front->command = (char*)malloc(1*sizeof(char*));
    front->arguments = (char**)malloc(1*sizeof(char*));
    front->numOfArguments = 0;
    front->inputRedirection = NULL;
    front->outputRedirection = NULL;
    front->isPipe = 0;
    front->pipeOutput = NULL;
}

void initializeOutputPipeStruct(struct outputPipeCommandInfo* pipeout){
    pipeout->path = NULL;
    pipeout->command = (char*)malloc(1*sizeof(char*));
    pipeout->arguments = (char**)malloc(1*sizeof(char*));
    pipeout->numOfArguments = 0;
    pipeout->inputRedirection = NULL;
    pipeout->outputRedirection = NULL;
    pipeout->input = NULL;
}


// ____________________________________________________________________________________________________________
//|                                           WILDCARD EXPAND                                                  |
//|____________________________________________________________________________________________________________|


char** myWildcard(char* input, char* path){
    numOfWildcards=1;
    //FIND WHERE *
    //set up
    int counter = 0;
    int position = 0;
    char *ptr = input;
    //loop to figure out where *
    while (*ptr != '*') {
        counter++;
        ptr++;
    }  
    //type of search established
    int y = strlen(input);
    if (counter == 0) {
        position = 2;
    } else if (y == counter+1) {
        position = 3;
    } else {
        position = 6;
    }
    //MAKE FIRST ARRAY  
    

    char *first = malloc(counter + 1);
    memcpy(first, input, counter);
    first[counter] = '\0';
    ptr++;
    //MAKE LAST ARRAY
    //figure out size of last
    int lcounter = 0;
    int lastsize = strlen(input) - counter - 1;
    char *last = malloc(lastsize + 1);
    while (*ptr != '\0') {
        last[lcounter] = *ptr;
        lcounter++;
        ptr++;
    }
    last[lcounter] = '\0';
    //MAKE MATCH ARRAY
    char** match = malloc(numOfWildcards * sizeof(char**));
    int i=0;
    int p=0;
   
    //MATCH algo
    struct dirent* zf;
    DIR* dp =opendir(path); //open current directory
    if(dp==NULL) {
        match[0] = malloc(sizeof(char) * (strlen(input)+1));
        match[0] = memcpy(match[0],input,sizeof(char) * strlen(input)+1);
        free(first);
        free(last);
        return match;
    }
    // CHECK FOR MATCHES
    while((zf=readdir(dp))!=NULL) {
        // printf("%d\n", zf->d_type);
        // printf("%s\n",zf ->d_name);
        if(position==3) {//match first
            int x = strncmp(first,(zf ->d_name), counter);
            if (x == 0) {
                match=realloc(match, numOfWildcards * sizeof(char *));
                // printf("IM not the problem\n");
                match[numOfWildcards-1] = malloc(sizeof(char) * (strlen(zf ->d_name)+1));
                // printf("IM not the problem\n");
                strcpy(match[p], (zf ->d_name));
                numOfWildcards++;    
                p++;
            }
            i++;
        }
        if(position==2) {
            int z = strlen((zf ->d_name));
            int lastn= z-lcounter;
            if(strncmp(".", (zf ->d_name),1)!=0) {
                int x=strncmp(last, (zf ->d_name)+lastn, lcounter);
                if (x == 0) {
                    match=realloc(match, numOfWildcards * sizeof(char *));
                    // printf("IM not the problem\n");
                    match[numOfWildcards-1] = malloc(sizeof(char) * (strlen(zf ->d_name)+1));
                    // printf("IM not the problem\n");
                    strcpy(match[p], (zf ->d_name));
                    numOfWildcards++;    
                    p++;
                }
                i++;
            }
        }
        if(position==6) {
            int z = strlen((zf ->d_name));
            int lastn= z-lcounter;
            int x=strncmp(last, (zf ->d_name)+lastn, lcounter);
            int y = strncmp(first, (zf ->d_name), counter);
            // printf("IM here\n");
            if (x == 0 && y== 0) {
                match=realloc(match, numOfWildcards * sizeof(char *));
                // printf("IM not the problem\n");
                match[numOfWildcards-1] = malloc(sizeof(char) * (strlen(zf ->d_name)+1));
                // printf("IM not the problem\n");
                strcpy(match[p], (zf ->d_name));
                numOfWildcards++;    
                p++;
            }
            i++;
        }
         
    }
    closedir(dp);
    free(first);
    free(last);
    
    // printf("list start here:\n");
    // for(int i=0; i <numOfWildcards-1;i++) {
    //     printf("%s\n",match[i]);
    // }
    
    if(numOfWildcards==1) {
        match[0] = malloc(sizeof(char) * (strlen(input)+1));
        match[0] = memcpy(match[0],input,sizeof(char) * strlen(input)+1);
        //printf("%s\n",match[0]);
        numOfWildcards++;
    }
    numOfWildcards--;
    return match;
}


// ____________________________________________________________________________________________________________
//|                                           WILDCARD PARSER                                                  |
//|____________________________________________________________________________________________________________|

char* wildcardParser(char* token){
    if(token[0] == '/'){
        char* fullinputpath = (char*)malloc((1+strlen(token))*sizeof(char));
        strcpy(fullinputpath,token);
        char* previouspath;
        char* wildcardword;
        int lastslash,recentslash;
        int check=0;
        int i=0;
        int totallength = strlen(fullinputpath);
        while(i<totallength){
            if((fullinputpath[i]=='/') && (check==0)){      //Remembers the last position of a /
                lastslash=i;
            }
            if((fullinputpath[i]=='/') && (check==1)){      //Here is the main part of it. 
                recentslash=i;
                wildcardword = (char*)malloc((recentslash-lastslash)*sizeof(char));
                for(int j=lastslash+1; j<recentslash;j++){
                    wildcardword[j-lastslash-1] = fullinputpath[j];
                }
                wildcardword[recentslash-lastslash-1] = '\0';
                //At this point, we have both the path and the name. We now need to call the function to get all the possibilities. 

                char** match=NULL;
                match=myWildcard(wildcardword,previouspath);
                char* expanded=match[0];

                char* remainingleft = (char*)malloc( (totallength-i+1)*sizeof(char));
                for(int j=recentslash; j<totallength; j++){
                    remainingleft[j-recentslash] = fullinputpath[j];
                }
                remainingleft[totallength-recentslash]='\0';
                //printf("length: %ld; remainingleft: %s\n",strlen(remainingleft),remainingleft);
                totallength = strlen(previouspath) + 1 + strlen(expanded) + strlen(remainingleft);
                //printf("totallength = %d\n",totallength);
                fullinputpath = (char*)realloc(fullinputpath,(totallength + 1)*(sizeof(char)));
                for(int j=0; j<strlen(previouspath); j++){
                    fullinputpath[j] = previouspath[j];
                }
                fullinputpath[strlen(previouspath) ] = '/';
                for(int j=0; j<strlen(expanded); j++){
                    fullinputpath[j+1+strlen(previouspath)] = expanded[j];
                }
                for(int j=0; j<strlen(remainingleft); j++){
                    fullinputpath[j+1+strlen(previouspath) + strlen(expanded)] = remainingleft[j];
                }
                fullinputpath[totallength]='\0';
                //printf("fullinputpath = %s\n",fullinputpath);
                i=i+(strlen(expanded)-strlen(wildcardword));
                lastslash=i;
                
                check=0;
                for(int y=0; y<numOfWildcards; y++){
                    free(match[y]);
                }
                free(match);
                free(previouspath);
                free(wildcardword);
                free(remainingleft);
            }
            if(fullinputpath[i]=='*'){
                check=1;
                previouspath = (char*)malloc(lastslash*(sizeof(char)) +1);
                memcpy(previouspath,fullinputpath,lastslash);
                previouspath[lastslash] = '\0';
                //printf("length = %lu, previouspath = %s\n",strlen(previouspath),previouspath);
            }
            i++;
        }
        if(check==1){       //The function for if it was at the end.
            wildcardword = (char*)malloc((totallength-lastslash)*sizeof(char));
            for(int j=lastslash+1; j<totallength;j++){
                wildcardword[j-lastslash-1] = fullinputpath[j];
            }
            wildcardword[totallength-lastslash-1] = '\0';
            //printf("length = %lu wildcardword = %s\n",strlen(wildcardword),wildcardword);
            
            char** match=NULL;
            match=myWildcard(wildcardword,previouspath);
            char* expanded=match[0];        
            
            totallength = strlen(previouspath) + 1 + strlen(expanded);
            fullinputpath = (char*)realloc(fullinputpath,(totallength + 1)*(sizeof(char)));
            for(int j=0; j<strlen(previouspath); j++){
                fullinputpath[j] = previouspath[j];
            }
            fullinputpath[strlen(previouspath) ] = '/';
            for(int j=0; j<strlen(expanded); j++){
                fullinputpath[j+1+strlen(previouspath)] = expanded[j];
            }
            fullinputpath[totallength]='\0';
            //printf("fullinputpath = %s\n",fullinputpath);

            //FREE EVERYTHING HERE
            for(int y=0; y<numOfWildcards; y++){
                free(match[y]);
            }
            free(match);
            free(wildcardword);
            free(previouspath);
            
            return fullinputpath;
        }
        else{
            return fullinputpath;
        }
    }

    char** match = myWildcard(token,"/usr/local/sbin");
    if( (numOfWildcards==1) && (strcmp(match[0],token)!=0) ){           //it is located in /usr/local/sbin
        char* bareName = (char*)malloc( strlen(match[0]) + 1 );
        strcpy(bareName,match[0]);

        for(int y=0; y<numOfWildcards; y++){
            free(match[y]);
        }
        free(match);

        return bareName;
    }
    for(int y=0; y<numOfWildcards; y++){
        free(match[y]);
    }
    free(match);

    match = myWildcard(token,"/usr/local/bin");
    if( (numOfWildcards==1) && (strcmp(match[0],token)!=0) ){           //it is located in /usr/local/sbin
        char* bareName = (char*)malloc( strlen(match[0]) + 1 );
        strcpy(bareName,match[0]);

        for(int y=0; y<numOfWildcards; y++){
            free(match[y]);
        }
        free(match);

        return bareName;
    }
    for(int y=0; y<numOfWildcards; y++){
        free(match[y]);
    }
    free(match);

    match = myWildcard(token,"/usr/sbin");
    if( (numOfWildcards==1) && (strcmp(match[0],token)!=0) ){           //it is located in /usr/local/sbin
        char* bareName = (char*)malloc( strlen(match[0]) + 1 );
        strcpy(bareName,match[0]);

        for(int y=0; y<numOfWildcards; y++){
            free(match[y]);
        }
        free(match);

        return bareName;
    }
    for(int y=0; y<numOfWildcards; y++){
        free(match[y]);
    }
    free(match);

    match = myWildcard(token,"/usr/bin");
    if( (numOfWildcards==1) && (strcmp(match[0],token)!=0) ){           //it is located in /usr/local/sbin
        char* bareName = (char*)malloc( strlen(match[0]) + 1 );
        strcpy(bareName,match[0]);

        for(int y=0; y<numOfWildcards; y++){
            free(match[y]);
        }
        free(match);

        return bareName;
    }
    for(int y=0; y<numOfWildcards; y++){
        free(match[y]);
    }
    free(match);

    match = myWildcard(token,"/sbin");
    if( (numOfWildcards==1) && (strcmp(match[0],token)!=0) ){           //it is located in /usr/local/sbin
        char* bareName = (char*)malloc( strlen(match[0]) + 1 );
        strcpy(bareName,match[0]);

        for(int y=0; y<numOfWildcards; y++){
            free(match[y]);
        }
        free(match);

        return bareName;
    }
    for(int y=0; y<numOfWildcards; y++){
        free(match[y]);
    }
    free(match);

    match = myWildcard(token,"/bin");
    if( (numOfWildcards==1) && (strcmp(match[0],token)!=0) ){           //it is located in /usr/local/sbin
        char* bareName = (char*)malloc( strlen(match[0]) + 1 );
        strcpy(bareName,match[0]);

        for(int y=0; y<numOfWildcards; y++){
            free(match[y]);
        }
        free(match);

        return bareName;
    }

    //If its none of the above

    for(int y=0; y<numOfWildcards; y++){
        free(match[y]);
    }
    free(match);


    return NULL;        //Only returns NULL if there are no matches at all
}



// ____________________________________________________________________________________________________________
//|                                            ANALYZE TOKEN                                                   |
//|____________________________________________________________________________________________________________|



void analyzeToken(struct commandInfo* front, char** tokens){
    int wildcardCheck = 0;          //Checks to see if we have a wildcard
    for(int i=0; i<strlen(tokens[0]); i++){         //Used to check if theres a wild card
        if(tokens[0][i] == '*'){
            wildcardCheck=1;
        }
    }

    if(wildcardCheck == 1){
        char* temp = wildcardParser(tokens[0]);
        if(temp==NULL){
            free(temp);
        }
        else if( strcmp(temp,tokens[0])!=0){          //If the two statements are not the same
            tokens[0] = (char*)realloc(tokens[0],strlen(temp)+1);
            memcpy(tokens[0],temp,strlen(temp)+1);
            free(temp); 
        }
        else{
            free(temp);
        }
        
    }
    
    //                                  Saving the first token in command
    front->command = (char*) realloc(front->command, (strlen(tokens[0]) + 1) * sizeof(char));           
    memcpy(front->command , tokens[0] , strlen(tokens[0]) + 1 );

    //                                  Saving the first token in arguments
    front->arguments[front->numOfArguments] = (char*) malloc( (strlen(tokens[0]) + 1) * sizeof(char));
    memcpy(front->arguments[front->numOfArguments] , tokens[0], strlen(tokens[0]) + 1);
    front->numOfArguments += 1;
    front->arguments = (char **) realloc(front->arguments, (front->numOfArguments + 1) * sizeof(char*) );



    
    if( (strcmp(front->command,"cd") == 0) || (strcmp(front->command,"pwd") == 0) || (strcmp(front->command,"exit") == 0) ){
        front->path = NULL;
    }

    else if( front->command[0] == '/'){      //setting the path for easy use
        front->path = (char*)malloc(strlen(front->command) +1 );
        strcpy(front->path,front->command);
    }

    else if( (front->command[0] == '.') && (front->command[1] == '/')){      //setting the path for easy use
        front->path = (char*)malloc(strlen(front->command) +1 );
        strcpy(front->path,front->command);
    }

    else{   //Barenames Path
        struct stat sb;
        char BareNames[100] = "/usr/local/sbin/";
        strcat(BareNames,front->command);
        int r=stat(BareNames,&sb);
        if(r==-1){
            char BareNames1[100] = "/usr/local/bin/";
            strcat(BareNames1,front->command);
            r=stat(BareNames1,&sb);
            if(r==-1){
                char BareNames2[100] = "/usr/sbin/";
                strcat(BareNames2,front->command);
                r=stat(BareNames2,&sb);
                if(r==-1){
                    char BareNames3[100] = "/usr/bin/";
                    strcat(BareNames3,front->command);
                    r=stat(BareNames3,&sb);
                    if(r==-1){
                        char BareNames4[100] = "/sbin/";
                        strcat(BareNames4,front->command);
                        r=stat(BareNames4,&sb);
                        if(r==-1){
                            char BareNames5[100] = "/bin/";
                            strcat(BareNames5,front->command);
                            r=stat(BareNames5,&sb);
                            if(r==-1){
                                front->path = NULL;
                            }
                            else{
                                front->path = (char*)malloc(strlen(BareNames5) +1 );
                                strcpy(front->path,BareNames5);
                            }
                        }
                        else{
                            front->path = (char*)malloc(strlen(BareNames4) +1 );
                            strcpy(front->path,BareNames4);
                        }
                    }
                    else{
                        front->path = (char*)malloc(strlen(BareNames3) +1 );
                        strcpy(front->path,BareNames3);
                    }
                }
                else{
                    front->path = (char*)malloc(strlen(BareNames2) +1 );
                    strcpy(front->path,BareNames2);
                }
            }
            else{
                front->path = (char*)malloc(strlen(BareNames1) +1 );
                strcpy(front->path,BareNames1);
            }
        }
        else{
            front->path = (char*)malloc(strlen(BareNames) +1 );
            strcpy(front->path,BareNames);
        }
    }





    int nextWordInput=0;
    int nextWordOutput=0;
    int i=1;
    while(i<numOfTokens){
        
        if( (strcmp(tokens[i],"<") != 0) && (strcmp(tokens[i],">") != 0) && (strcmp(tokens[i],"|") != 0) && (nextWordInput == 0)  && (nextWordOutput == 0) ){    //This is a normal word
            wildcardCheck=0;
            for(int j=0; j<strlen(tokens[i]);j++){
                if(tokens[i][j] == '*'){
                    wildcardCheck = 1;
                    
                }
            }
            if(wildcardCheck == 1){
                if(tokens[i][0] == '/'){
                    
                    char* temp = wildcardParser(tokens[i]);
                    front->arguments[front->numOfArguments] = (char*) malloc( (strlen(temp) + 1) * sizeof(char));
                    memcpy(front->arguments[front->numOfArguments] , temp, strlen(temp) + 1);
                    front->numOfArguments += 1;
                    front->arguments = (char **) realloc(front->arguments, (front->numOfArguments + 1) * sizeof(char*) );
                    free(temp); 
                }
                else{
                    
                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));
                    char**match = myWildcard(tokens[i],cwd);
                    
                    for(int j=0; j<numOfWildcards; j++){
                        front->arguments[front->numOfArguments] = (char*) malloc( (strlen(match[j]) + 1) * sizeof(char));
                        memcpy(front->arguments[front->numOfArguments] , match[j], strlen(match[j]) + 1);
                        front->numOfArguments += 1;
                        front->arguments = (char **) realloc(front->arguments, (front->numOfArguments + 1) * sizeof(char*) );
                        free(match[j]);
                    }
                    free(match);
                }

            }
            if(wildcardCheck == 0){
                front->arguments[front->numOfArguments] = (char*) malloc( (strlen(tokens[i]) + 1) * sizeof(char));
                memcpy(front->arguments[front->numOfArguments] , tokens[i], strlen(tokens[i]) + 1);
                front->numOfArguments += 1;
                front->arguments = (char **) realloc(front->arguments, (front->numOfArguments + 1) * sizeof(char*) );
            }
        }
        
        else if( (strcmp(tokens[i],"<") != 0) && (strcmp(tokens[i],">") != 0) && (strcmp(tokens[i],"|") != 0) && (nextWordInput == 1) && (front->inputRedirection != NULL) ){    //This is a INPUT word
            printf("ERROR: THERE IS ALREADY AN INPUT REDIRECTION. NOT VALID\n");
            errorcheck=1;
            break;
        }

        else if( (strcmp(tokens[i],"<") != 0) && (strcmp(tokens[i],">") != 0) && (strcmp(tokens[i],"|") != 0) && (nextWordOutput == 1) && (front->outputRedirection != NULL) ){    //This is a INPUT word
            printf("ERROR: THERE IS ALREADY AN OUTPUT REDIRECTION. NOT VALID\n");
            errorcheck=1;
            break;
        }

        else if( (strcmp(tokens[i],"<") != 0) && (strcmp(tokens[i],">") != 0) && (strcmp(tokens[i],"|") != 0) && (nextWordInput == 1) ){    //This is a INPUT word
            wildcardCheck=0;
            for(int j=0; j<strlen(tokens[i]);j++){
                if(tokens[i][j] == '*'){
                    wildcardCheck = 1;
                }
            }
            if(wildcardCheck == 1){
                if(tokens[i][0] == '/'){
                    char* temp = wildcardParser(tokens[i]);
                    front->arguments[front->numOfArguments] = (char*) malloc( (strlen(temp) + 1) * sizeof(char));
                    memcpy(front->arguments[front->numOfArguments] , temp, strlen(temp) + 1);
                    front->numOfArguments += 1;
                    front->arguments = (char **) realloc(front->arguments, (front->numOfArguments + 1) * sizeof(char*) );
                    free(temp); 
                }
                else{
                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));
                    char**match = myWildcard(tokens[i],cwd);
                    
                    for(int j=0; j<numOfWildcards; j++){
                        front->arguments[front->numOfArguments] = (char*) malloc( (strlen(match[j]) + 1) * sizeof(char));
                        memcpy(front->arguments[front->numOfArguments] , match[j], strlen(match[j]) + 1);
                        front->numOfArguments += 1;
                        front->arguments = (char **) realloc(front->arguments, (front->numOfArguments + 1) * sizeof(char*) );
                        free(match[j]);
                    }
                    free(match);
                }
            }
            else if(wildcardCheck == 0){
                front->inputRedirection = (char*) malloc( (strlen(tokens[i]) + 1) * sizeof(char));
                memcpy(front->inputRedirection , tokens[i], strlen(tokens[i]) + 1);
                nextWordInput=0;
            }
        }

        else if( (strcmp(tokens[i],"<") != 0) && (strcmp(tokens[i],">") != 0) && (strcmp(tokens[i],"|") != 0) && (nextWordOutput == 1) ){    //This is a OUTPUT word
            wildcardCheck=0;
            for(int j=0; j<strlen(tokens[i]);j++){
                if(tokens[i][j] == '*'){
                    wildcardCheck = 1;
                }
            }
            if(wildcardCheck == 1){
                if(tokens[i][0] == '/'){
                    char* temp = wildcardParser(tokens[i]);
                    front->arguments[front->numOfArguments] = (char*) malloc( (strlen(temp) + 1) * sizeof(char));
                    memcpy(front->arguments[front->numOfArguments] , temp, strlen(temp) + 1);
                    front->numOfArguments += 1;
                    front->arguments = (char **) realloc(front->arguments, (front->numOfArguments + 1) * sizeof(char*) );
                    free(temp); 
                }
                else{
                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));
                    char**match = myWildcard(tokens[i],cwd);
                    
                    for(int j=0; j<numOfWildcards; j++){
                        front->arguments[front->numOfArguments] = (char*) malloc( (strlen(match[j]) + 1) * sizeof(char));
                        memcpy(front->arguments[front->numOfArguments] , match[j], strlen(match[j]) + 1);
                        front->numOfArguments += 1;
                        front->arguments = (char **) realloc(front->arguments, (front->numOfArguments + 1) * sizeof(char*) );
                        free(match[j]);
                    }
                    free(match);
                }
            }
            else if(wildcardCheck == 0){
                front->outputRedirection = (char*) malloc( (strlen(tokens[i]) + 1) * sizeof(char));
                memcpy(front->outputRedirection , tokens[i], strlen(tokens[i]) + 1);
                nextWordOutput=0;
            }
            
        }

        else if( ( (strcmp(tokens[i],"<") == 0) || (strcmp(tokens[i],">") == 0) || (strcmp(tokens[i],"|") == 0) ) && ( ( (nextWordOutput == 1) ) || ((nextWordInput == 1) ) ) ){
            printf("ERROR: YOU NEED TO FINISH YOUR INPUT OR OUTPUT REDIRECTION\n");
            errorcheck=1;
            break;
        }

        else if( strcmp(tokens[i],"|") == 0 ){
            if(front->outputRedirection != NULL){
                printf("ERROR: YOU CAN'T HAVE BOTH OUTPUT REDIRECTION AND PIPE\n");
                errorcheck=1;
                break;
            }
            front->isPipe = 1;      //this tells us that we are going to have a pipe
            break;
        }

        else if( strcmp(tokens[i],"<") == 0  ){     //This lets us know that the word is INPUT redirection
            nextWordInput=1;
        }

        else if( strcmp(tokens[i],">") == 0  ){           //This lets us know that the word is Output redirection
            nextWordOutput=1;
        }
        i++;
    }
    front->arguments[front->numOfArguments] = NULL;         //Ensures that the last argument is a NULL argument
    if( ( nextWordInput == 1 ) || (nextWordOutput == 1) ){
        errorcheck = 1;
        printf("ERROR: YOU NEED TO FINISH YOUR INPUT OR OUTPUT REDIRECTION\n");
    }
    if(errorcheck == 1){
        return;
    }

    // printf("\n\nPath: %s\n",front->path);
    // printf("Command: %s\n",front->command);
    // printf("number of arugments: %d\n",front->numOfArguments);
    // for(int i=0; i<=front->numOfArguments; i++){
    //     printf("Argument %d: %s, ",i +1 ,front->arguments[i]);
    // }
    // printf("\nInputRedirect: %s\n",front->inputRedirection);
    // printf("OutputRedirect: %s\n",front->outputRedirection);

    // printf("i=%d numOfTokens = %d\n",i,numOfTokens);

    if(i==numOfTokens){
        return;
    }




    //From here on out, only the pipe stuff gets saved





    if( front->isPipe == 1){
        if( i == numOfTokens-1){
            printf("ERROR: YOU NEED TO PIPE TO SOMETHING\n");
            errorcheck = 1;
            return;
        }
    }
    struct outputPipeCommandInfo* pipeInfo = (struct outputPipeCommandInfo*)malloc(1*sizeof(struct outputPipeCommandInfo));
    initializeOutputPipeStruct(pipeInfo);
    front->pipeOutput = pipeInfo;       //Make sure that it points to the struct
    pipeInfo->input = front;
    i++;
    
    wildcardCheck = 0;          //Checks to see if we have a wildcard
    for(int j=0; j<strlen(tokens[i]); j++){         //Used to check if theres a wild card
        if(tokens[i][j] == '*'){
            wildcardCheck=1;
        }
    }

    if(wildcardCheck == 1){
        char* temp = wildcardParser(tokens[i]);
        if(temp==NULL){
            free(temp);
        }
        else if( strcmp(temp,tokens[i])!=0){          //If the two statements are not the same
            tokens[i] = (char*)realloc(tokens[i],strlen(temp)+1);
            memcpy(tokens[i],temp,strlen(temp)+1);
            free(temp);             //ERROR MIGHT NEED TO REMOVE LATER
        }
        else{
            free(temp);             //ERROR MIGHT NEED TO REMOVE LATER
        }
        
    }
    
    pipeInfo->command = (char*) realloc(pipeInfo->command, (strlen(tokens[i]) + 1) * sizeof(char));           
    memcpy(pipeInfo->command , tokens[i] , strlen(tokens[i]) + 1 );

    //                                  Saving the first token in arguments
    pipeInfo->arguments[pipeInfo->numOfArguments] = (char*) malloc( (strlen(tokens[i]) + 1) * sizeof(char));
    memcpy(pipeInfo->arguments[pipeInfo->numOfArguments] , tokens[i], strlen(tokens[i]) + 1);
    pipeInfo->numOfArguments += 1;
    pipeInfo->arguments = (char **) realloc(pipeInfo->arguments, (pipeInfo->numOfArguments + 1) * sizeof(char*) );




    if( (strcmp(pipeInfo->command,"cd") == 0) || (strcmp(pipeInfo->command,"pwd") == 0) || (strcmp(pipeInfo->command,"exit") == 0) ){
        pipeInfo->path = NULL;
    }

    else if( pipeInfo->command[0] == '/'){      //setting the path for easy use
        pipeInfo->path = (char*)malloc(strlen(pipeInfo->command) +1 );
        strcpy(pipeInfo->path,pipeInfo->command);
    }

    else if( (pipeInfo->command[0] == '.') && (pipeInfo->command[1] == '/')){      //setting the path for easy use
        pipeInfo->path = (char*)malloc(strlen(pipeInfo->command) +1 );
        strcpy(pipeInfo->path,pipeInfo->command);
    }

    else{   //Barenames Path
        struct stat sb;
        char BareNames[100] = "/usr/local/sbin/";
        strcat(BareNames,pipeInfo->command);
        int r=stat(BareNames,&sb);
        if(r==-1){
            char BareNames1[100] = "/usr/local/bin/";
            strcat(BareNames1,pipeInfo->command);
            r=stat(BareNames1,&sb);
            if(r==-1){
                char BareNames2[100] = "/usr/sbin/";
                strcat(BareNames2,pipeInfo->command);
                r=stat(BareNames2,&sb);
                if(r==-1){
                    char BareNames3[100] = "/usr/bin/";
                    strcat(BareNames3,pipeInfo->command);
                    r=stat(BareNames3,&sb);
                    if(r==-1){
                        char BareNames4[100] = "/sbin/";
                        strcat(BareNames4,pipeInfo->command);
                        r=stat(BareNames4,&sb);
                        if(r==-1){
                            char BareNames5[100] = "/bin/";
                            strcat(BareNames5,pipeInfo->command);
                            r=stat(BareNames5,&sb);
                            if(r==-1){
                                pipeInfo->path = NULL;
                            }
                            else{
                                pipeInfo->path = (char*)malloc(strlen(BareNames5) +1 );
                                strcpy(pipeInfo->path,BareNames5);
                            }
                        }
                        else{
                            pipeInfo->path = (char*)malloc(strlen(BareNames4) +1 );
                            strcpy(pipeInfo->path,BareNames4);
                        }
                    }
                    else{
                        pipeInfo->path = (char*)malloc(strlen(BareNames3) +1 );
                        strcpy(pipeInfo->path,BareNames3);
                    }
                }
                else{
                    pipeInfo->path = (char*)malloc(strlen(BareNames2) +1 );
                    strcpy(pipeInfo->path,BareNames2);
                }
            }
            else{
                pipeInfo->path = (char*)malloc(strlen(BareNames1) +1 );
                strcpy(pipeInfo->path,BareNames1);
            }
        }
        else{
            pipeInfo->path = (char*)malloc(strlen(BareNames) +1 );
            strcpy(pipeInfo->path,BareNames);
        }
    }



    i++;

    nextWordOutput=0;
    while(i<numOfTokens){
        if( strcmp(tokens[i],"<") == 0  ){     //This lets us know that the word is INPUT redirection
            printf("ERROR: YOU CAN NOT HAVE AN INPUT REDIRECTION INSIDE OF A PIPE\n");
            errorcheck=1;
            break;
        }

        else if( strcmp(tokens[i],"|") == 0 ){
            printf("ERROR: THERE IS NO MULTIPLE PIPES EXTENSION. YOU CAN NOT DO MULTIPLE PIPES\n");
            errorcheck=1;
            break;
        }
        
        else if( (strcmp(tokens[i],">") != 0) && (nextWordOutput == 0) ){    //This is a normal word
            wildcardCheck=0;
            for(int j=0; j<strlen(tokens[i]);j++){
                if(tokens[i][j] == '*'){
                    wildcardCheck = 1;
                }
            }
            if(wildcardCheck == 1){
                if(tokens[i][0] == '/'){
                    char* temp = wildcardParser(tokens[i]);
                    pipeInfo->arguments[pipeInfo->numOfArguments] = (char*) malloc( (strlen(temp) + 1) * sizeof(char));
                    memcpy(pipeInfo->arguments[pipeInfo->numOfArguments] , temp, strlen(temp) + 1);
                    pipeInfo->numOfArguments += 1;
                    pipeInfo->arguments = (char **) realloc(pipeInfo->arguments, (pipeInfo->numOfArguments + 1) * sizeof(char*) );
                    free(temp); 
                }
                else{
                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));
                    char**match = myWildcard(tokens[i],cwd);
                    for(int j=0; j<numOfWildcards; j++){
                        pipeInfo->arguments[pipeInfo->numOfArguments] = (char*) malloc( (strlen(match[j]) + 1) * sizeof(char));
                        memcpy(pipeInfo->arguments[pipeInfo->numOfArguments] , match[j], strlen(match[j]) + 1);
                        pipeInfo->numOfArguments += 1;
                        pipeInfo->arguments = (char **) realloc(pipeInfo->arguments, (pipeInfo->numOfArguments + 1) * sizeof(char*) );
                        free(match[j]);
                    }
                    free(match);
                }
                

            }
            if(wildcardCheck == 0){
                pipeInfo->arguments[pipeInfo->numOfArguments] = (char*) malloc( (strlen(tokens[i]) + 1) * sizeof(char));
                memcpy(pipeInfo->arguments[pipeInfo->numOfArguments] , tokens[i], strlen(tokens[i]) + 1);
                pipeInfo->numOfArguments += 1;
                pipeInfo->arguments = (char **) realloc(pipeInfo->arguments, (pipeInfo->numOfArguments + 1) * sizeof(char*) );
            }
        }
        

        else if( (strcmp(tokens[i],">") != 0) && (nextWordOutput == 1) && (pipeInfo->outputRedirection != NULL) ){    //This is a INPUT word
            printf("ERROR: THERE IS ALREADY AN OUTPUT REDIRECTION. NOT VALID\n");
            errorcheck=1;
            break;
        }

        else if( (strcmp(tokens[i],">") != 0) && (nextWordOutput == 1) ){    //This is a OUTPUT word
            wildcardCheck=0;
            for(int j=0; j<strlen(tokens[i]);j++){
                if(tokens[i][j] == '*'){
                    wildcardCheck = 1;
                }
            }
            if(wildcardCheck == 1){
                if(tokens[i][0] == '/'){
                    char* temp = wildcardParser(tokens[i]);
                    pipeInfo->arguments[pipeInfo->numOfArguments] = (char*) malloc( (strlen(temp) + 1) * sizeof(char));
                    memcpy(pipeInfo->arguments[pipeInfo->numOfArguments] , temp, strlen(temp) + 1);
                    pipeInfo->numOfArguments += 1;
                    pipeInfo->arguments = (char **) realloc(pipeInfo->arguments, (pipeInfo->numOfArguments + 1) * sizeof(char*) );
                    free(temp); 
                }
                else{
                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));
                    char**match = myWildcard(tokens[i],cwd);
                    for(int j=0; j<numOfWildcards; j++){
                        pipeInfo->arguments[pipeInfo->numOfArguments] = (char*) malloc( (strlen(match[j]) + 1) * sizeof(char));
                        memcpy(pipeInfo->arguments[pipeInfo->numOfArguments] , match[j], strlen(match[j]) + 1);
                        pipeInfo->numOfArguments += 1;
                        pipeInfo->arguments = (char **) realloc(pipeInfo->arguments, (pipeInfo->numOfArguments + 1) * sizeof(char*) );
                        free(match[j]);
                    }
                    free(match);
                }
            }
            else if(wildcardCheck == 0){
                pipeInfo->outputRedirection = (char*) malloc( (strlen(tokens[i]) + 1) * sizeof(char));
                memcpy(pipeInfo->outputRedirection , tokens[i], strlen(tokens[i]) + 1);
                nextWordOutput=0;
            }
            
        }

        else if( (strcmp(tokens[i],">") == 0) && (nextWordOutput == 1) ){
            printf("ERROR: YOU NEED TO FINISH YOUR OUTPUT REDIRECTION\n");
            errorcheck=1;
            break;
        }

        else if( strcmp(tokens[i],">") == 0  ){           //This lets us know that the word is Output redirection
            nextWordOutput=1;
        }
        i++;
    }
    pipeInfo->arguments[pipeInfo->numOfArguments] = NULL;         //Ensures that the last argument is a NULL argument
    
    if( nextWordOutput == 1 ){
        errorcheck = 1;
        printf("ERROR: YOU NEED TO FINISH YOUR INPUT OR OUTPUT REDIRECTION\n");
    }
    if(errorcheck == 1){
        return;
    }    
    
    // printf("\n\nPath: %s\n",pipeInfo->path);
    // printf("Command: %s\n",pipeInfo->command);
    // printf("number of arugments: %d\n",pipeInfo->numOfArguments);
    // for(int j=0; j<pipeInfo->numOfArguments; j++){
    //     printf("Argument %d: %s, ",pipeInfo->numOfArguments ,pipeInfo->arguments[j]);
    // }
    // printf("\nInputRedirect: %s\n",pipeInfo->inputRedirection);
    // printf("OutputRedirect: %s\n",pipeInfo->outputRedirection);

}



// ____________________________________________________________________________________________________________
//|                                           INTERACTIVE LOOP                                                 |
//|____________________________________________________________________________________________________________|



void shellLoop(){
    //printf("Welcome to My Shell(mysh)!\n");
    write(STDOUT_FILENO,"Welcome to My Shell(mysh)\n",strlen("Welcome to My Shell(mysh)\n"));
    
    while(1){
        if(errorcheck==1){
            write(STDOUT_FILENO,"!",strlen("!"));
        }
        errorcheck=0;
        write(STDOUT_FILENO,"mysh> ",strlen("mysh> "));
        // printf("mysh> ");
        // fflush(stdout);
        char buffer[MAXBUFFER];
        BytesRead = read(STDIN_FILENO,buffer,MAXBUFFER);
        
        char **tokens = parsing(buffer);

        struct commandInfo front;
        initializeCommandStruct(&front);
        analyzeToken(&front,tokens);

        if(errorcheck == 1){
            freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
            continue;
        }


        // ______________________________________________________
        //|     At this point, we are going to check the         |
        //|          commands and figure out what to do          |
        //|______________________________________________________|


        // ______________________________________________________
        //|                  EXIT CONDITION                      |
        //|______________________________________________________|

        if( (strcmp(front.command,"exit") == 0) && (front.isPipe == 0) ){                  //exit condition
            if((front.numOfArguments == 1) && (front.isPipe == 0)){
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                break;
            }
            else{
                printf("exit: too many arguments\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                continue;
            }
        }

        // ______________________________________________________
        //|                         PWD                          |
        //|______________________________________________________|
        
        else if( (strcmp(front.command,"pwd") == 0) && (front.isPipe == 0) ){   
            if(front.numOfArguments == 1){
                if(front.inputRedirection != NULL){
                    printf("ERROR: Can not Redirect Input with pwd\n");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck = 1;
                    continue;
                }

                char cwd[512];
                getcwd(cwd, sizeof(cwd));
                if(front.outputRedirection != NULL){            //If we have an Output Redirection
                    int fd = open(front.outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                    if (fd == -1) {
                        perror("open");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        continue;
                    }

                    pid_t pid = fork();
        
                    if (pid == -1) {
                        perror("fork");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        continue;
                    }

                    if(pid == 0){

                        if (dup2(fd, STDOUT_FILENO) == -1) {
                            perror("dup2");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            errorcheck=1;
                            continue;
                        }
                        printf("%s\n", cwd);

                        exit(EXIT_SUCCESS);
                    }

                    if (wait(NULL) == -1) {
                        perror("wait");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        continue;
                    }

                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    continue;
                }
                
                printf("%s\n", cwd);
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                continue;
            }
            else{
                printf("pwd: too many arguments\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                continue;
            }
        }

        // ______________________________________________________
        //|                         CD                           |
        //|______________________________________________________|
        
        else if( (strcmp(front.command,"cd") == 0) ){
            if(front.isPipe == 0){      //if there is no pipe
                char* newdir;
                if(numOfTokens == 1) {
                    newdir="~";
                }
                else {
                    newdir=front.arguments[1];
                }

                if(strncmp(newdir,"~/",2)==0) {
                    newdir+=2;
                    char* home= getenv("HOME");
                    chdir(home);
                }
                //char cwd[512];
                if(strcmp(newdir,"~")==0) {
                    char* home= getenv("HOME");
                    chdir(home);
                }
                else {
                    int i=chdir(newdir);
                    if(i ==-1) {
                        perror("Error");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        continue;
                    }
                }
                //getcwd(cwd, sizeof(cwd));           //testing 
                //printf("%s\n", cwd);                //testing
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                continue;
            }
            else if(front.isPipe == 1){
                printf("ERROR: We can not have CD in a pipe due to undefined Behavior\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                continue;
            }
        }
        // ______________________________________________________
        //|                      PATHS                           |
        //|______________________________________________________|

        else if( (front.path != NULL) && (front.isPipe == 0) ){
            int outputfd;
            int inputfd;
            if(front.outputRedirection != NULL){
                outputfd = open(front.outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                if (outputfd == -1) {
                    perror("open");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    continue;
                }
            }
            
            if(front.inputRedirection != NULL){
                inputfd = open(front.inputRedirection,O_RDONLY);
                if (inputfd == -1) {
                    perror("open");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    continue;
                }
            }

            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                continue;
            }

            if(pid == 0){
                if(front.outputRedirection!=NULL){
                    if (dup2(outputfd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        continue;
                    }
                }
                if(front.inputRedirection!=NULL){
                    if (dup2(inputfd, STDIN_FILENO) == -1) {
                        perror("dup2");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        continue;
                    }
                }
                
                execv(front.path,front.arguments);

                perror("execv");
                errorcheck=1;
                abort();
            }
            int wstatus;
            
            if ( wait(&wstatus) == -1) {
                perror("wait");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                continue;
            }
            if (WEXITSTATUS(wstatus) != 0) {
                printf("ERROR WITH WAIT\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                continue;
            }


            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            continue;

        }
        
        // ______________________________________________________
        //|                       PIPING                         |
        //|______________________________________________________|

        if(front.isPipe == 1){

        //printf("We are now piping\n");

        if( (strcmp(front.command,"exit") == 0) || (strcmp(front.pipeOutput->command,"exit") == 0) ){       //If we have exit as one of our commands
            //printf("We are now piping\n");
            if(strcmp(front.command,"exit") == 0){          //Exit is on the left, so we will execute the right side
                if(front.numOfArguments >1){
                    printf("exit: too many arguments\n");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    continue;
                }

                // ______________________________________________________
                //|                         PWD                          |
                //|______________________________________________________|
                
                if( (strcmp(front.pipeOutput->command,"pwd") == 0)  ){   
                    if(front.pipeOutput->numOfArguments == 1){
                        char cwd[512];
                        getcwd(cwd, sizeof(cwd));
                        if(front.pipeOutput->outputRedirection != NULL){            //If we have an Output Redirection
                            int fd = open(front.pipeOutput->outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                            if (fd == -1) {
                                perror("open");
                                freeing(tokens);        //used to free the tokens and reset for the loop
                                if(front.isPipe == 1){
                                    pipestructfreeing(front.pipeOutput);
                                }
                                firststructfreeing(&front);
                                break;
                            }

                            pid_t pid = fork();
                
                            if (pid == -1) {
                                perror("fork");
                                freeing(tokens);        //used to free the tokens and reset for the loop
                                if(front.isPipe == 1){
                                    pipestructfreeing(front.pipeOutput);
                                }
                                firststructfreeing(&front);
                                break;
                            }

                            if(pid == 0){

                                if (dup2(fd, STDOUT_FILENO) == -1) {
                                    perror("dup2");
                                    freeing(tokens);        //used to free the tokens and reset for the loop
                                    if(front.isPipe == 1){
                                        pipestructfreeing(front.pipeOutput);
                                    }
                                    firststructfreeing(&front);
                                    break;
                                }
                                printf("%s\n", cwd);

                                exit(EXIT_SUCCESS);
                            }

                            if (wait(NULL) == -1) {
                                perror("wait");
                                freeing(tokens);        //used to free the tokens and reset for the loop
                                if(front.isPipe == 1){
                                    pipestructfreeing(front.pipeOutput);
                                }
                                firststructfreeing(&front);
                                break;
                            }

                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            break;
                        }
                        
                        printf("%s\n", cwd);
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        break;
                    }
                    else{
                        printf("pwd: too many arguments");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        break;
                    }
                }


                // ______________________________________________________
                //|                      PATHS                           |
                //|______________________________________________________|

                else if( (front.pipeOutput->path != NULL) ){
                    int outputfd;
                    if(front.pipeOutput->outputRedirection != NULL){
                        outputfd = open(front.pipeOutput->outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                        if (outputfd == -1) {
                            perror("open");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            errorcheck=1;
                            break;
                        }
                    }
                    

                    pid_t pid = fork();

                    if (pid == -1) {
                        perror("fork");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        break;
                    }

                    if(pid == 0){
                        if(front.pipeOutput->outputRedirection!=NULL){
                            if (dup2(outputfd, STDOUT_FILENO) == -1) {
                                perror("dup2");
                                freeing(tokens);        //used to free the tokens and reset for the loop
                                if(front.isPipe == 1){
                                    pipestructfreeing(front.pipeOutput);
                                }
                                firststructfreeing(&front);
                                errorcheck=1;
                                break;
                            }
                        }
                        
                        
                        execv(front.pipeOutput->path,front.pipeOutput->arguments);

                        perror("execv");
                        errorcheck=1;
                        abort();
                    }
                    int wstatus;
                    
                    if ( wait(&wstatus) == -1) {
                        perror("wait");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        break;
                    }
                    if (WEXITSTATUS(wstatus) != 0) {
                        printf("ERROR WITH WAIT\n");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        break;
                    }


                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    break;

                }




            }
            
            
            else if(strcmp(front.pipeOutput->command,"exit") == 0){                   //Exit is on the right, so we will execute the left side
                if(front.pipeOutput->numOfArguments >1){
                    printf("exit: too many arguments\n");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    continue;
                }



                // ______________________________________________________
                //|                         PWD                          |
                //|______________________________________________________|
                
                if( (strcmp(front.command,"pwd") == 0) ){   
                    if(front.numOfArguments == 1){
                        if(front.inputRedirection != NULL){
                            printf("ERROR: Can not Redirect Input with pwd\n");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            errorcheck = 1;
                            break;
                        }

                        char cwd[512];
                        getcwd(cwd, sizeof(cwd));
                        printf("%s\n", cwd);
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        break;
                    }
                    else{
                        printf("pwd: too many arguments");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        break;
                    }
                }


                // ______________________________________________________
                //|                      PATHS                           |
                //|______________________________________________________|

                else if( (front.path != NULL) ){
                    int inputfd;
                    
                    if(front.inputRedirection != NULL){
                        inputfd = open(front.inputRedirection,O_RDONLY);
                        if (inputfd == -1) {
                            perror("open");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            errorcheck=1;
                            break;
                        }
                    }

                    pid_t pid = fork();

                    if (pid == -1) {
                        perror("fork");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        break;
                    }

                    if(pid == 0){
                        if(front.inputRedirection!=NULL){
                            if (dup2(inputfd, STDIN_FILENO) == -1) {
                                perror("dup2");
                                freeing(tokens);        //used to free the tokens and reset for the loop
                                if(front.isPipe == 1){
                                    pipestructfreeing(front.pipeOutput);
                                }
                                firststructfreeing(&front);
                                errorcheck=1;
                                break;
                            }
                        }
                        
                        execv(front.path,front.arguments);

                        perror("execv");
                        errorcheck=1;
                        abort();
                    }
                    int wstatus;
                    
                    if ( wait(&wstatus) == -1) {
                        perror("wait");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        break;
                    }
                    if (WEXITSTATUS(wstatus) != 0) {
                        printf("ERROR WITH WAIT\n");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        break;
                    }


                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    break;

                }



            }

            errorcheck = 1;
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            break;

        }

            else if ( ((strcmp(front.command,"pwd") == 0) || (front.path != NULL)) && ((strcmp(front.pipeOutput->command,"pwd") == 0) || (front.pipeOutput->path != NULL)) ){                   //This is our general case piping
                int inputfd;
                int outputfd;
                    
                if(front.inputRedirection != NULL){
                    inputfd = open(front.inputRedirection,O_RDONLY);
                    if (inputfd == -1) {
                        perror("open");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        continue;
                    }
                }

                if(front.pipeOutput->outputRedirection != NULL){
                    outputfd = open(front.pipeOutput->outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                    if (outputfd == -1) {
                        perror("open");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        continue;
                    }
                }


                int pipefd[2];
                pipe(pipefd);

                pid_t cpid = fork();
                if (cpid == -1) {
                    perror("fork");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    continue;
                }
                if(cpid==0){
                    close(pipefd[0]);       //Read End
                    dup2(pipefd[1],STDOUT_FILENO);


                    if(front.inputRedirection!=NULL){
                        if (dup2(inputfd, STDIN_FILENO) == -1) {
                            perror("dup2");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            errorcheck=1;
                            continue;
                        }
                    }


                    //EXEC HERE
                    if(strcmp(front.command,"pwd") == 0){
                        char cwd[512];
                        getcwd(cwd, sizeof(cwd));           //testing 
                        write(pipefd[1],cwd,strlen(cwd) + 1);
                        abort();
                    }
                    else if(front.path != NULL){
                        execv(front.path,front.arguments);
                        perror("execv");
                        errorcheck=1;
                        abort();
                    }
                    abort();
                }

                cpid=fork();
                if (cpid == -1) {
                    perror("fork");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    continue;
                }
                if(cpid==0){
                    close(pipefd[1]);       //Close Write End
                    dup2(pipefd[0],STDIN_FILENO);

                    if(front.pipeOutput->outputRedirection!=NULL){
                        if (dup2(outputfd, STDOUT_FILENO) == -1) {
                            perror("dup2");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            errorcheck=1;
                            continue;
                        }
                    }

                    if(strcmp(front.pipeOutput->command,"pwd") == 0){
                        char cwd[512];
                        getcwd(cwd, sizeof(cwd));           //testing 
                        write(pipefd[1],cwd,strlen(cwd) + 1);
                        abort();
                    }
                    else if(front.pipeOutput->path != NULL){
                        execv(front.pipeOutput->path,front.pipeOutput->arguments);
                        perror("execv");
                        errorcheck=1;
                        abort();
                    }
                    abort();
                }
                close(pipefd[0]);       //Read End
                close(pipefd[1]);       //Close Write End


                int status;
                int temp = waitpid(-1,&status,0);
                if(temp == -1){
                    perror("wait");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    continue;
                }
                if (WEXITSTATUS(status) != 0) {
                    printf("ERROR WITH WAIT\n");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    continue;
                }
                

                temp = waitpid(-1,&status,0);
                if(temp == -1){
                    perror("wait");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    continue;
                }
                if (WEXITSTATUS(status) != 0) {
                    printf("ERROR WITH WAIT\n");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    continue;
                }

                //Exit normally

                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                continue;
            }
            printf("ERROR: Invalid Command\n");
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            errorcheck = 1;
            continue;
        }

        // ______________________________________________________
        //|           END OF COMMANDS. FREE BY DEFAULT           |
        //|______________________________________________________|

        printf("ERROR: Invalid Command\n");
        errorcheck = 1;
        freeing(tokens);        //used to free the tokens and reset for the loop
        if(front.isPipe == 1){
            pipestructfreeing(front.pipeOutput);
        }
        firststructfreeing(&front);
    }
}



void myBatchExecuting(char* sentence){
    //write(STDOUT_FILENO,"$ ", strlen("$ "));
    printf("$ %s \n",sentence);
    BytesRead = strlen(sentence) + 1;
    char **tokens = parsing(sentence);

    struct commandInfo front;
    initializeCommandStruct(&front);
    analyzeToken(&front,tokens);


    // ______________________________________________________
    //|     At this point, we are going to check the         |
    //|          commands and figure out what to do          |
    //|______________________________________________________|


    // ______________________________________________________
    //|                  EXIT CONDITION                      |
    //|______________________________________________________|

    if( (strcmp(front.command,"exit") == 0) && (front.isPipe == 0) ){                  //exit condition
        if((front.numOfArguments == 1) && (front.isPipe == 0)){
            batchExit = 1;
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            return;
        }
        else{
            printf("exit: too many arguments\n");
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            errorcheck=1;
            return;
        }
    }

    // ______________________________________________________
    //|                         PWD                          |
    //|______________________________________________________|
    
    else if( (strcmp(front.command,"pwd") == 0) && (front.isPipe == 0) ){   
        if(front.numOfArguments == 1){
            if(front.inputRedirection != NULL){
                printf("ERROR: Can not Redirect Input with pwd\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck = 1;
                return;
            }

            char cwd[512];
            getcwd(cwd, sizeof(cwd));
            if(front.outputRedirection != NULL){            //If we have an Output Redirection
                int fd = open(front.outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                if (fd == -1) {
                    perror("open");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }

                pid_t pid = fork();
    
                if (pid == -1) {
                    perror("fork");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }

                if(pid == 0){

                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        return;
                    }
                    printf("%s\n", cwd);

                    exit(EXIT_SUCCESS);
                }

                if (wait(NULL) == -1) {
                    perror("wait");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }

                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                return;
            }
            else{
                printf("%s\n", cwd);
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                return;
            }
        }
        else{
            printf("pwd: too many arguments\n");
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            errorcheck=1;
            return;
        }
    }

    // ______________________________________________________
    //|                         CD                           |
    //|______________________________________________________|
    
    else if( (strcmp(front.command,"cd") == 0) ){
        if(front.isPipe == 0){
            char* newdir;
            if(numOfTokens == 1) {
                newdir="~";
            }
            else {
                newdir=front.arguments[1];
            }

            if(strncmp(newdir,"~/",2)==0) {
                newdir+=2;
                char* home= getenv("HOME");
                chdir(home);
            }
            //char cwd[512];
            if(strcmp(newdir,"~")==0) {
                char* home= getenv("HOME");
                chdir(home);
            }
            else {
                int i=chdir(newdir);
                if(i ==-1) {
                    perror("Error");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    return;
                }
            }
            //getcwd(cwd, sizeof(cwd));           //testing 
            //printf("%s\n", cwd);                //testing
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            printf("\n");
            return;
        }
        else if(front.isPipe == 1){
            printf("ERROR: We can not have CD in a pipe due to undefined Behavior\n");
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            return;
        }
    }
    // ______________________________________________________
    //|                      PATHS                           |
    //|______________________________________________________|

    else if( (front.path != NULL) && (front.isPipe == 0) ){
        int outputfd;
        int inputfd;
        if(front.outputRedirection != NULL){
            outputfd = open(front.outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
            if (outputfd == -1) {
                perror("open");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                return;
            }
        }
        
        if(front.inputRedirection != NULL){
            inputfd = open(front.inputRedirection,O_RDONLY);
            if (inputfd == -1) {
                perror("open");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                return;
            }
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            errorcheck=1;
            return;
        }

        if(pid == 0){
            if(front.outputRedirection!=NULL){
                if (dup2(outputfd, STDOUT_FILENO) == -1) {
                    perror("dup2");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }
            }
            if(front.inputRedirection!=NULL){
                if (dup2(inputfd, STDIN_FILENO) == -1) {
                    perror("dup2");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }
            }
            
            execv(front.path,front.arguments);

            perror("execv");
            errorcheck=1;
            abort();
        }
        int wstatus;
        
        if ( wait(&wstatus) == -1) {
            perror("wait");
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            errorcheck=1;
            return;
        }
        if (WEXITSTATUS(wstatus) != 0) {
            printf("ERROR WITH WAIT\n");
            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            errorcheck=1;
            return;
        }


        freeing(tokens);        //used to free the tokens and reset for the loop
        if(front.isPipe == 1){
            pipestructfreeing(front.pipeOutput);
        }
        firststructfreeing(&front);
        return;

    }
    
    // ______________________________________________________
    //|                       PIPING                         |
    //|______________________________________________________|

    if(front.isPipe == 1){

    //printf("We are now piping\n");

    if( (strcmp(front.command,"exit") == 0) || (strcmp(front.pipeOutput->command,"exit") == 0) ){       //If we have exit as one of our commands
        //printf("We are now piping\n");
        batchExit = 1;
        if(strcmp(front.command,"exit") == 0){          //Exit is on the left, so we will execute the right side
            if(front.numOfArguments >1){
                printf("exit: too many arguments\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                return;
            }

            // ______________________________________________________
            //|                         PWD                          |
            //|______________________________________________________|
            
            if( (strcmp(front.pipeOutput->command,"pwd") == 0)  ){   
                if(front.pipeOutput->numOfArguments == 1){
                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));
                    if(front.pipeOutput->outputRedirection != NULL){            //If we have an Output Redirection
                        int fd = open(front.pipeOutput->outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                        if (fd == -1) {
                            perror("open");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            return;
                        }

                        pid_t pid = fork();
            
                        if (pid == -1) {
                            perror("fork");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            return;
                        }

                        if(pid == 0){

                            if (dup2(fd, STDOUT_FILENO) == -1) {
                                perror("dup2");
                                freeing(tokens);        //used to free the tokens and reset for the loop
                                if(front.isPipe == 1){
                                    pipestructfreeing(front.pipeOutput);
                                }
                                firststructfreeing(&front);
                                return;
                            }
                            printf("%s\n", cwd);

                            exit(EXIT_SUCCESS);
                        }

                        if (wait(NULL) == -1) {
                            perror("wait");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            return;
                        }

                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        return;
                    }
                    
                    printf("%s\n", cwd);
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    return;
                }
                else{
                    printf("pwd: too many arguments");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    return;
                }
            }


            // ______________________________________________________
            //|                      PATHS                           |
            //|______________________________________________________|

            else if( (front.pipeOutput->path != NULL) ){
                int outputfd;
                if(front.pipeOutput->outputRedirection != NULL){
                    outputfd = open(front.pipeOutput->outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                    if (outputfd == -1) {
                        perror("open");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        return;
                    }
                }
                

                pid_t pid = fork();

                if (pid == -1) {
                    perror("fork");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }

                if(pid == 0){
                    if(front.pipeOutput->outputRedirection!=NULL){
                        if (dup2(outputfd, STDOUT_FILENO) == -1) {
                            perror("dup2");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            errorcheck=1;
                            return;
                        }
                    }
                    
                    
                    execv(front.pipeOutput->path,front.pipeOutput->arguments);

                    perror("execv");
                    errorcheck=1;
                    abort();
                }
                int wstatus;
                
                if ( wait(&wstatus) == -1) {
                    perror("wait");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }
                if (WEXITSTATUS(wstatus) != 0) {
                    printf("ERROR WITH WAIT\n");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }


                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                return;

            }




        }
        
        
        else if(strcmp(front.pipeOutput->command,"exit") == 0){                   //Exit is on the right, so we will execute the left side
            if(front.pipeOutput->numOfArguments >1){
                printf("exit: too many arguments\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                return;
            }



            // ______________________________________________________
            //|                         PWD                          |
            //|______________________________________________________|
            
            if( (strcmp(front.command,"pwd") == 0) ){   
                if(front.numOfArguments == 1){
                    if(front.inputRedirection != NULL){
                        printf("ERROR: Can not Redirect Input with pwd\n");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck = 1;
                        return;
                    }

                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));
                    printf("%s\n", cwd);
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    return;
                }
                else{
                    printf("pwd: too many arguments");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    return;
                }
            }


            // ______________________________________________________
            //|                      PATHS                           |
            //|______________________________________________________|

            else if( (front.path != NULL) ){
                int inputfd;
                
                if(front.inputRedirection != NULL){
                    inputfd = open(front.inputRedirection,O_RDONLY);
                    if (inputfd == -1) {
                        perror("open");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        return;
                    }
                }

                pid_t pid = fork();

                if (pid == -1) {
                    perror("fork");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }

                if(pid == 0){
                    if(front.inputRedirection!=NULL){
                        if (dup2(inputfd, STDIN_FILENO) == -1) {
                            perror("dup2");
                            freeing(tokens);        //used to free the tokens and reset for the loop
                            if(front.isPipe == 1){
                                pipestructfreeing(front.pipeOutput);
                            }
                            firststructfreeing(&front);
                            errorcheck=1;
                            return;
                        }
                    }
                    
                    execv(front.path,front.arguments);

                    perror("execv");
                    errorcheck=1;
                    abort();
                }
                int wstatus;
                
                if ( wait(&wstatus) == -1) {
                    perror("wait");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }
                if (WEXITSTATUS(wstatus) != 0) {
                    printf("ERROR WITH WAIT\n");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }


                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                return;

            }



        }

        errorcheck = 1;
        freeing(tokens);        //used to free the tokens and reset for the loop
        if(front.isPipe == 1){
            pipestructfreeing(front.pipeOutput);
        }
        firststructfreeing(&front);
        return;

    }

        else if ( ((strcmp(front.command,"pwd") == 0) || (front.path != NULL)) && ((strcmp(front.pipeOutput->command,"pwd") == 0) || (front.pipeOutput->path != NULL)) ){                   //This is our general case piping
            int inputfd;
            int outputfd;
                
            if(front.inputRedirection != NULL){
                inputfd = open(front.inputRedirection,O_RDONLY);
                if (inputfd == -1) {
                    perror("open");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }
            }

            if(front.pipeOutput->outputRedirection != NULL){
                outputfd = open(front.pipeOutput->outputRedirection, O_CREAT | O_TRUNC | O_WRONLY,0640);
                if (outputfd == -1) {
                    perror("open");
                    freeing(tokens);        //used to free the tokens and reset for the loop
                    if(front.isPipe == 1){
                        pipestructfreeing(front.pipeOutput);
                    }
                    firststructfreeing(&front);
                    errorcheck=1;
                    return;
                }
            }


            int pipefd[2];
            pipe(pipefd);

            pid_t cpid = fork();
            if (cpid == -1) {
                perror("fork");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                return;
            }
            if(cpid==0){
                close(pipefd[0]);       //Read End
                dup2(pipefd[1],STDOUT_FILENO);


                if(front.inputRedirection!=NULL){
                    if (dup2(inputfd, STDIN_FILENO) == -1) {
                        perror("dup2");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        return;
                    }
                }


                //EXEC HERE
                if(strcmp(front.command,"pwd") == 0){
                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));           //testing 
                    write(pipefd[1],cwd,strlen(cwd) + 1);
                    abort();
                }
                else if(front.path != NULL){
                    execv(front.path,front.arguments);
                    perror("execv");
                    errorcheck=1;
                    abort();
                }
                abort();
            }

            cpid=fork();
            if (cpid == -1) {
                perror("fork");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                return;
            }
            if(cpid==0){
                close(pipefd[1]);       //Close Write End
                dup2(pipefd[0],STDIN_FILENO);

                if(front.pipeOutput->outputRedirection!=NULL){
                    if (dup2(outputfd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        freeing(tokens);        //used to free the tokens and reset for the loop
                        if(front.isPipe == 1){
                            pipestructfreeing(front.pipeOutput);
                        }
                        firststructfreeing(&front);
                        errorcheck=1;
                        return;
                    }
                }

                if(strcmp(front.pipeOutput->command,"pwd") == 0){
                    char cwd[512];
                    getcwd(cwd, sizeof(cwd));           //testing 
                    write(pipefd[1],cwd,strlen(cwd) + 1);
                    abort();
                }
                else if(front.pipeOutput->path != NULL){
                    execv(front.pipeOutput->path,front.pipeOutput->arguments);
                    perror("execv");
                    errorcheck=1;
                    abort();
                }
                abort();
            }
            close(pipefd[0]);       //Read End
            close(pipefd[1]);       //Close Write End


            int status;
            int temp = waitpid(-1,&status,0);
            if(temp == -1){
                perror("wait");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                return;
            }
            if (WEXITSTATUS(status) != 0) {
                printf("ERROR WITH WAIT\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                return;
            }
            

            temp = waitpid(-1,&status,0);
            if(temp == -1){
                perror("wait");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                return;
            }
            if (WEXITSTATUS(status) != 0) {
                printf("ERROR WITH WAIT\n");
                freeing(tokens);        //used to free the tokens and reset for the loop
                if(front.isPipe == 1){
                    pipestructfreeing(front.pipeOutput);
                }
                firststructfreeing(&front);
                errorcheck=1;
                return;
            }

            //Exit normally

            freeing(tokens);        //used to free the tokens and reset for the loop
            if(front.isPipe == 1){
                pipestructfreeing(front.pipeOutput);
            }
            firststructfreeing(&front);
            return;
        }
        printf("ERROR: Invalid Command\n");
        freeing(tokens);        //used to free the tokens and reset for the loop
        if(front.isPipe == 1){
            pipestructfreeing(front.pipeOutput);
        }
        firststructfreeing(&front);
        errorcheck = 1;
        return;
    }

    // ______________________________________________________
    //|           END OF COMMANDS. FREE BY DEFAULT           |
    //|______________________________________________________|

    printf("ERROR: Invalid Command\n");
    errorcheck = 1;
    freeing(tokens);        //used to free the tokens and reset for the loop
    if(front.isPipe == 1){
        pipestructfreeing(front.pipeOutput);
    }
    firststructfreeing(&front);




}



void batchMatch(char* argv){
    char *buffer = malloc(2*sizeof(char));
    int fd = open(argv,O_RDONLY);
    if(fd==-1){
        return;
    }
    int BytesRead = read(fd,buffer,1);
    int size=1;
    char *sentence  = malloc(sizeof(char));
    sentence[0]='\0';
    
    while(BytesRead>0){
        buffer[1]='\0';
        if(buffer[0] != '\n'){
            size++;
            sentence = realloc(sentence,sizeof(char)*(size+1));
            strcat(sentence,buffer);
        }
        else{
            sentence=realloc(sentence,sizeof(char)*(size));
            sentence[size-1] = '\0';
            myBatchExecuting(sentence);
            if(batchExit == 1){
                free(sentence);
                free(buffer);
                return;
            }
            size=1;
            free(sentence);
            sentence = malloc(sizeof(char));
            sentence[0]='\0';
            
        }
        BytesRead=read(fd,buffer,1);
    }
    sentence=realloc(sentence,sizeof(char)*(size));
    sentence[size-1] = '\0';
    myBatchExecuting(sentence);
    free(sentence);
    free(buffer);
}

int main( int argc, char *argv[] ){    
    if(argc==1){
        shellLoop();
    }
    else if(argc == 2){
        
        batchMatch(argv[1]);
    }

    else{
        printf("You have provided too many arguments\n");
    }
    
    return 0;
}