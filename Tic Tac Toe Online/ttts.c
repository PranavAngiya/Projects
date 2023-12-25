// NOTE: must use option -pthread when compiling!
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#define QUEUE_SIZE 8

int numOfGames=0;

volatile int active = 1;

void handler(int signum){
    active = 0;
}

void install_handlers(sigset_t *mask){
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    
    sigemptyset(mask);
    sigaddset(mask, SIGINT);
    sigaddset(mask, SIGTERM);
}

int open_listener(char *service, int queue_size){
    struct addrinfo hint, *info_list, *info;
    int error, sock;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family   = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags    = AI_PASSIVE;

    // obtain information for listening socket
    error = getaddrinfo(NULL, service, &hint, &info_list);
    if (error) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    // attempt to create socket
    for (info = info_list; info != NULL; info = info->ai_next) {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

        // if we could not create the socket, try the next method
        if (sock == -1) continue;

        // bind socket to requested port
        error = bind(sock, info->ai_addr, info->ai_addrlen);
        if (error) {
            close(sock);
            continue;
        }

        // enable listening for incoming connection requests
        error = listen(sock, queue_size);
        if (error) {
            close(sock);
            continue;
        }

        // if we got this far, we have opened the socket
        break;
    }

    freeaddrinfo(info_list);

    // info will be NULL if no method succeeded
    if (info == NULL) {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    return sock;
}

#define BUFSIZE 256
#define HOSTSIZE 100
#define PORTSIZE 10


struct playerInfo{
    int sockID;
    char* name;             //Name of the player
    char role;              //Role of the player: X or O
};

void initStruct(struct playerInfo* players){
    players->name = NULL;             //Name of the player
}


// ____________________________________________________________________________________________________________
//|                                                BOARD LOGIC                                                 |
//|____________________________________________________________________________________________________________|



bool verti(int vert, char player,char board[3][3]) {
    int counter=0;
    int h=0;
    while(h<=2) {
        if(board[vert][h]==player){
            counter++;
        }
        h++;
    }
    if(counter==3){
        return true;
    }
    return false;
}

bool horiz(int hori, char player, char board[3][3]){
    int counter=0;
    int v=0;
    while(v <=2) {
        if(board[v][hori]==player){
            counter++;
        }
        v++;
    }
    if(counter==3){
        return true;
    }
    return false;
}

bool dia(char player, char board[3][3]){
    if(board[0][0]==player &&board[2][2]==player) {
        return true;
    }
    else if(board[0][2]==player &&board[2][0]==player) {
        return true;
    }
    return false; 

}

bool draw(char board[3][3]){
int counter=0; 
    for(int j=0; j<3; j++){
                    for(int k=0; k<3; k++){
                        if(board[j][k] != 'E') {
                            counter++;
                        }
                    }

}
if (counter==9){
    return true;
}
return false;
}
 
int game(int hori, int vert, char player, char board[3][3]){
    if(hori >3|| vert > 3) {
        return 2; //INVALID POSITION DOESNT EXIST
    }
    hori--;
    vert--;
    if(board[vert][hori]=='E') {
        board[vert][hori]=player;
        bool winner;
        winner= verti(vert,player,board);
        if (winner==true) {
            if(player=='X') {
                return 3;       //If X wins,
            }
            else {
                return 4;       //If O wins
            }
        }    
        winner= horiz(hori,player,board);
        if (winner==true) {
                if(player=='X') {
                    return 3;       //If X wins
                }
                else {
                    return 4;       //If O wins
                }
        }
        if( board[1][1]==player){
            winner= dia(player, board);
            if (winner==true) {
                if(player=='X') {
                    return 3;       //If X wins
                }
                else {
                    return 4;       //If O wins
                }
            }
        }
        if(draw(board)==true){
            return 5;               //If its a draw
        }
        return 1;                   //Valid move, no win or draw yet
    }
    return 2;                       //Choose a position that's in use
}



// ____________________________________________________________________________________________________________
//|                                                 MAIN GAME                                                  |
//|____________________________________________________________________________________________________________|




void *read_data(void *arg){

    int numGames=numOfGames;

	struct playerInfo *players = (struct playerInfo*) arg;
    char temp1[20];                 //Used to store the length of the package message
    char temp2[20];                 //Used to store the length of the package message
    snprintf(temp1,20,"%d",1+1+ ( (int)strlen(players[1].name) )+1);           //Used to convert the integer value of the package message into a string
    snprintf(temp2,20,"%d",1+1+ ( (int)strlen(players[0].name) )+1);           //Used to convert the integer value of the package message into a string
    int messageLength1 = (int)strlen(temp1);                                  //Find the length of the string by using strlen
    int messageLength2 = (int)strlen(temp2);                                  //Find the length of the string by using strlen
    int finalLength1 = 4+1+messageLength1+1+1+1+ ( (int)strlen(players[1].name) )+1 + 1;      //4 for MOVD, 1 for |,messageLength for length of remaining message, 1 for |, 1 for W, 1 for |, x for stringLength of name, 10 for " has Won!|" + 1 for NULL terminator
    int finalLength2 = 4+1+messageLength2+1+1+1+ ( (int)strlen(players[0].name) )+1 + 1;      //4 for MOVD, 1 for |,messageLength for length of remaining message, 1 for |, 1 for W, 1 for |, x for stringLength of name, 10 for " has Won!|" + 1 for NULL terminator
    char message1[finalLength1];                                              //Message to send to client
    char message2[finalLength2];                                              //Message to send to client
    snprintf(message1,finalLength1,"BEGN|%d|X|%s|",2 +  ( (int)strlen(players[1].name) )+1,players[1].name);
    snprintf(message2,finalLength2,"BEGN|%d|O|%s|",2 +  ( (int)strlen(players[0].name) )+1,players[0].name);
    write(players[0].sockID,message1,strlen(message1)+1);
    write(players[1].sockID,message2,strlen(message2)+1);

    // ____________________________________________________________________________________________________________
    //|                                            GETTING CLIENT INFO                                             |
    //|____________________________________________________________________________________________________________|

    char buf[BUFSIZE + 1];
    int bytes;

    // ____________________________________________________________________________________________________________
    //|                                              SETTING UP BOARD                                              |
    //|____________________________________________________________________________________________________________|



    char board[3][3]= {
        {'E','E','E'},
        {'E','E','E'},
        {'E','E','E'}
    };

    int currentPlayer=0;


    // ____________________________________________________________________________________________________________
    //|                                                 MAIN LOOP                                                  |
    //|____________________________________________________________________________________________________________|



    while (active && (bytes = read(players[currentPlayer].sockID, buf, BUFSIZE)) > 0) {

        printf("Message Recieved from Player %d: '%s'\n", currentPlayer+1,buf);

        int other;
        if(currentPlayer == 0){
            other = 1;
        }
        else{
            other = 0;
        }


        // ____________________________________________________________________________________________________________
        //|                                              FULL MESSAGE                                                  |
        //|____________________________________________________________________________________________________________|

        char *start = strchr(buf,'|') + 1;      //Gets the pointer to the first | in the received message
        char *end = strchr(start,'|');          //Gets the pointer to the second | in the received message to get the integer length
        int distance = end - start;
        char fieldLen[distance + 1];
        strncpy(fieldLen,start,distance);
        fieldLen[distance] = '\0';
        int fieldLength = atoi(fieldLen);           //What is the length of the message sent

        if( (bytes - 4 - 1 - distance - 1 - fieldLength) > 0){          //If we don't recieve the entire message
            write(players[currentPlayer].sockID,"INVL|18|!Invalid Message!|",strlen("INVL|18|!Invalid Message!|") + 1);
            write(players[other].sockID,"INVL|45|!Invalid Message from Opponent! Ending game!|",strlen("INVL|45|!Invalid Message from Opponent! Ending game!|") + 1);
            break;
        }

        if( buf[4] != '|'){     //PLAY|10|Joe Smith|
            write(players[currentPlayer].sockID,"INVL|18|!Invalid Message!|",strlen("INVL|18|!Invalid Message!|") + 1);
            write(players[other].sockID,"INVL|45|!Invalid Message from Opponent! Ending game!|",strlen("INVL|45|!Invalid Message from Opponent! Ending game!|") + 1);
            break;
        }

        // ____________________________________________________________________________________________________________
        //|                                                UNPACK MESSAGE                                              |
        //|____________________________________________________________________________________________________________|

        char action[5];
        memcpy(action,buf,4);
        action[4] = '\0';

        if(players[currentPlayer].name == NULL){           //If the player doesn't exist: They never entered PLAY
            write(players[0].sockID,"INVL|20|Never joined a game|",strlen("INVL|20|Never joined a game|"));
            write(players[1].sockID,"INVL|20|Never joined a game|",strlen("INVL|20|Never joined a game|"));
            break;
        }

        if( strcmp(action,"PLAY") == 0){        //If our action word is PLAY
            write(players[currentPlayer].sockID,"INVL|25|Already playing the game|",strlen("INVL|25|Already playing the game|"));
            continue;
        }

        else if( strcmp(action,"MOVE") == 0){
            if(fieldLength != 6){           //The Move is invalid either because the integer value is greater or there's some error
                write(players[currentPlayer].sockID,"INVL|26|!INVALID PROTOCOL MESSAGE|",strlen("INVL|26|!INVALID PROTOCOL MESSAGE|") + 1);
                write(players[currentPlayer].sockID,"INVL|40|!INVALID PROTOCOL MESSAGE FROM OPPONENT|",strlen("INVL|40|!INVALID PROTOCOL MESSAGE FROM OPPONENT|") + 1);
                break;
            }
            char player = *(end+1);     //Stores the player who is playing

            end = strchr(end,'|');          //Moves end to the next | for the horizontal and vertical Value
            char temp[2];                   //Variable used to get the integer information
            temp[0] = *(end+3);
            temp[1] = '\0';
            int horizontal = atoi(temp);         //To convert it into an integer
            temp[0] = *(end+5); 
            int vertical = atoi(temp);
            if( (vertical <= 0 ) || (vertical >= 4 ) || (horizontal <= 0 ) || (horizontal >= 4 ) ){
                write(players[currentPlayer].sockID,"INVL|21|NOT A VALID POSITION|",strlen("INVL|21|NOT A VALID POSITION|") + 1);
                continue;
            }

            int output = game(horizontal,vertical,player,board);            //Places the position onto the board and sees it's outcome

            if(output == 1){                //If the position was a valid move
                char message[25];
                char boardMessage[10];
                int counter=0;
                for(int i = 0; i < 3; i++){
                    for(int j=0; j<3; j++){
                        if(board[i][j] == 'E'){
                            boardMessage[counter] = '.';
                            counter++;
                        }
                        else if(board[i][j] == 'X'){
                            boardMessage[counter] = 'X';
                            counter++;
                        }
                        else{
                            boardMessage[counter] = 'O';
                            counter++;
                        }
                        printf("%c ",board[i][j]);
                    }
                    printf("\n");
                }

                boardMessage[9] = '\0';
                snprintf(message,25,"MOVD|16|%c|%d,%d|%s|",player,horizontal,vertical,boardMessage);
                write(players[0].sockID,message,25);
                write(players[1].sockID,message,25);
                
                currentPlayer = other;
                
                continue;

            }
            else if(output == 2){                //The position is in use
                write(players[currentPlayer].sockID,"INVL|24|THAT SPACE IS OCCUPIED.|",strlen("INVL|24|THAT SPACE IS OCCUPIED.|") + 1);
                continue;
            }
            else if(output == 3){                //X wins
                char temp[20];              //Used to store the length of the package message
                snprintf(temp,20,"%d",1+1+ ( (int)strlen(players[0].name) )+10);           //Used to convert the integer value of the package message into a string
                int messageLength = (int)strlen(temp);                                  //Find the length of the string by using strlen
                int finalLength = 4+1+messageLength+1+1+1+ ( (int)strlen(players[0].name) )+10 + 1;      //4 for MOVD, 1 for |,messageLength for length of remaining message, 1 for |, 1 for W, 1 for |, x for stringLength of name, 10 for " has Won!|" + 1 for NULL terminator
                char message1[finalLength];                                              //Message to send to client
                char message2[finalLength];                                              //Message to send to client
                snprintf(message1,finalLength,"OVER|%d|W|%s has Won!|",2 +  ( (int)strlen(players[0].name) )+10,players[0].name);
                snprintf(message2,finalLength,"OVER|%d|L|%s has Won!|",2 +  ( (int)strlen(players[0].name) )+10,players[0].name);
                write(players[0].sockID,message1,strlen(message1)+1);
                write(players[1].sockID,message2,strlen(message2)+1);
                break;
            }
            else if(output == 4){                //O wins
                char temp[20];              //Used to store the length of the package message
                snprintf(temp,20,"%d",1+1+ ( (int)strlen(players[1].name) )+10);           //Used to convert the integer value of the package message into a string
                int messageLength = (int)strlen(temp);                                  //Find the length of the string by using strlen
                int finalLength = 4+1+messageLength+1+1+1+ ( (int)strlen(players[1].name) )+10 + 1;      //4 for MOVD, 1 for |,messageLength for length of remaining message, 1 for |, 1 for W, 1 for |, x for stringLength of name, 10 for " has Won!|" + 1 for NULL terminator
                char message1[finalLength];                                              //Message to send to client
                char message2[finalLength];                                              //Message to send to client
                snprintf(message1,finalLength,"OVER|%d|L|%s has Won!|",2 +  ( (int)strlen(players[1].name) )+10,players[1].name);
                snprintf(message2,finalLength,"OVER|%d|W|%s has Won!|",2 +  ( (int)strlen(players[1].name) )+10,players[1].name);
                write(players[0].sockID,message1,strlen(message1)+1);
                write(players[1].sockID,message2,strlen(message2)+1);
                break;
            }
            else if(output == 5){                //DRAW
                write(players[0].sockID,"OVER|25|D|Nobody won! Good Game!|",strlen("OVER|25|D|Nobody won! Good Game!|") + 1);
                write(players[1].sockID,"OVER|25|D|Nobody won! Good Game!|",strlen("OVER|25|D|Nobody won! Good Game!|") + 1);
                break;
            }

            continue;
        }
        
        else if( strcmp(action,"RSGN") == 0){       //If a player requests to resign
            if(fieldLength != 0 ){              //Dangerous Protocall error when recieving RSGN with another argument
                write(players[currentPlayer].sockID,"INVL|18|!Invalid Message!|",strlen("INVL|18|!Invalid Message!|") + 1);
                write(players[other].sockID,"INVL|45|!Invalid Message from Opponent! Ending game!|",strlen("INVL|45|!Invalid Message from Opponent! Ending game!|") + 1);
                break;
            }

            char temp[20];              //Used to store the length of the package message
            snprintf(temp,20,"%d",1+1+ ( (int)strlen(players[currentPlayer].name) )+15);           //Used to convert the integer value of the package message into a string
            int messageLength = (int)strlen(temp);                                  //Find the length of the string by using strlen
            int finalLength = 4+1+messageLength+1+1+1+ ( (int)strlen(players[currentPlayer].name) )+15 + 1;      //4 for MOVD, 1 for |,messageLength for length of remaining message, 1 for |, 1 for W, 1 for |, x for stringLength of name, 10 for " has Won!|" + 1 for NULL terminator
            
            
            char message1[finalLength];                                              //Message to send to client
            char message2[finalLength];                                              //Message to send to client
            snprintf(message1,finalLength,"OVER|%d|W|%s has resigned.|",2 +  ( (int)strlen(players[currentPlayer].name) )+15,players[currentPlayer].name);             //32
            snprintf(message2,finalLength,"OVER|%d|L|%s has resigned.|",2 +  ( (int)strlen(players[currentPlayer].name) )+15,players[currentPlayer].name);             //32
            message1[finalLength-1] = '\0';
            message2[finalLength-1] = '\0';
            
            
            if(currentPlayer == 1){         //If O resigns                
                write(players[0].sockID,message1,strlen(message1)+1);
                write(players[1].sockID,message2,strlen(message2)+1);
                break;
            }

            else{                           //If X resigns
                write(players[0].sockID,message2,strlen(message2)+1);
                write(players[1].sockID,message1,strlen(message1)+1);
                break;
            }
        }
        
        else if( strcmp(action,"DRAW") == 0){           //If a player requests a draw
            if(fieldLength != 2){           //Dangerous Protocall error with additional arguments
                write(players[currentPlayer].sockID,"INVL|18|!Invalid Message!|",strlen("INVL|18|!Invalid Message!|") + 1);
                write(players[other].sockID,"INVL|45|!Invalid Message from Opponent! Ending game!|",strlen("INVL|45|!Invalid Message from Opponent! Ending game!|") + 1);
                break;
            }
            
            char drawAction = *(end+1);     //Used to store the action of draw

            if(drawAction == 'S'){
                //send the draw action to other person

                // bytes = write(players[other].sockID,buf,bytes);
                char sentencemessage[10];
                memcpy(sentencemessage,buf,9);
                sentencemessage[9] = '\0';
                
                write(players[other].sockID,sentencemessage,10);

                //read the input

                if( (bytes = read(players[other].sockID,buf,BUFSIZE) ) <= 0){         //If the message can't be read, this is a potential error and must be terminated
                    write(players[currentPlayer].sockID,"INVL|30|!UNABLE TO READ DRAW RESPONSE|",strlen("INVL|30|!UNABLE TO READ DRAW RESPONSE|") + 1);
                    write(players[other].sockID,"INVL|30|!UNABLE TO READ DRAW RESPONSE|",strlen("INVL|30|!UNABLE TO READ DRAW RESPONSE|") + 1);
                    break;
                }

                //Error checks

                start = strchr(buf,'|') + 1;      //Gets the pointer to the first | in the received message
                end = strchr(start,'|');          //Gets the pointer to the second | in the received message to get the integer length
                distance = end - start;
                fieldLen[distance + 1];
                strncpy(fieldLen,start,distance);
                fieldLen[distance] = '\0';
                fieldLength = atoi(fieldLen);           //What is the length of the message sent

                if( (bytes - 4 - 1 - distance - 1 - fieldLength) > 0){          //If we don't recieve the entire message
                    write(players[currentPlayer].sockID,"INVL|18|!Invalid Message!|",strlen("INVL|18|!Invalid Message!|") + 1);
                    write(players[other].sockID,"INVL|45|!Invalid Message from Opponent! Ending game!|",strlen("INVL|45|!Invalid Message from Opponent! Ending game!|") + 1);
                    break;
                }

                char action[5];
                memcpy(action,buf,4);
                action[4] = '\0';

                if(strcmp(action,"DRAW") == 0){
                    if(fieldLength != 2){
                        write(players[currentPlayer].sockID,"INVL|18|!Invalid Message!|",strlen("INVL|18|!Invalid Message!|") + 1);
                        write(players[other].sockID,"INVL|45|!Invalid Message from Opponent! Ending game!|",strlen("INVL|45|!Invalid Message from Opponent! Ending game!|") + 1);
                        break;
                    }
                    else{
                        drawAction = *(end+1);
                    }
                }

                else{           //If the command isn't valid
                    do{
                        write(players[other].sockID,"INVL|46|NOT A VALID COMMAND. MUST USE A DRAW RESPONSE|",strlen("INVL|46|NOT A VALID COMMAND. MUST USE A DRAW RESPONSE|") + 1);
                        bytes = read(players[other].sockID,buf,BUFSIZE);

                        start = strchr(buf,'|') + 1;      //Gets the pointer to the first | in the received message
                        end = strchr(start,'|');          //Gets the pointer to the second | in the received message to get the integer length
                        distance = end - start;
                        fieldLen[distance + 1];
                        strncpy(fieldLen,start,distance);
                        fieldLen[distance] = '\0';
                        fieldLength = atoi(fieldLen);           //What is the length of the message sent

                        char action[5];
                        memcpy(action,buf,4);
                        action[4] = '\0';
                        
                    } while ( strcmp(action,"DRAW") != 0 );

                    drawAction = *(end+1);

                }


                //if accept, issue draw
                if(drawAction == 'A'){
                    write(players[0].sockID,"OVER|25|D|Nobody won! Good Game!|",strlen("OVER|25|D|Nobody won! Good Game!|") + 1);
                    write(players[1].sockID,"OVER|25|D|Nobody won! Good Game!|",strlen("OVER|25|D|Nobody won! Good Game!|") + 1);
                    break;
                }

                //if reject, continue
                else if(drawAction == 'R'){
                    sentencemessage[10];
                    memcpy(sentencemessage,buf,9);
                    sentencemessage[9] = '\0';

                    write(players[currentPlayer].sockID, sentencemessage, 10);
                    continue;
                }
                else{
                    
                    do{
                        write(players[other].sockID,"INVL|20|NOT A VALID COMMAND|",strlen("INVL|20|NOT A VALID COMMAND|") + 1);
                        bytes = read(players[other].sockID,buf,BUFSIZE);

                        start = strchr(buf,'|') + 1;      //Gets the pointer to the first | in the received message
                        end = strchr(start,'|');          //Gets the pointer to the second | in the received message to get the integer length
                        distance = end - start;
                        fieldLen[distance + 1];
                        strncpy(fieldLen,start,distance);
                        fieldLen[distance] = '\0';
                        fieldLength = atoi(fieldLen);           //What is the length of the message sent

                        char action[5];
                        memcpy(action,buf,4);
                        action[4] = '\0';

                        drawAction = *(end+1);
                        
                    } while ( (drawAction != 'R') || (drawAction != 'A') );

                    if(drawAction == 'A'){
                        write(players[0].sockID,"OVER|25|D|Nobody won! Good Game!|",strlen("OVER|25|D|Nobody won! Good Game!|") + 1);
                        write(players[1].sockID,"OVER|25|D|Nobody won! Good Game!|",strlen("OVER|25|D|Nobody won! Good Game!|") + 1);
                        break;
                    }

                    //if reject, continue
                    else if(drawAction == 'R'){
                        sentencemessage[10];
                        memcpy(sentencemessage,buf,9);
                        sentencemessage[9] = '\0';

                        write(players[currentPlayer].sockID, sentencemessage, 10);
                        continue;
                    }

                    
                }
            }
        
            else{
                write(players[currentPlayer].sockID,"INVL|35|NO DRAW REQUEST HAS BEEN MADE YET!|",strlen("INVL|35|NO DRAW REQUEST HAS BEEN MADE YET!|") + 1);
                continue;
            }
        }
        
        else{
            write(players[currentPlayer].sockID,"INVL|20|NOT A VALID COMMAND|",strlen("INVL|20|NOT A VALID COMMAND|") + 1);
            continue;
        }

    }


	if (bytes == 0) {                                   //Current player disconnects
		printf("Got EOF\n");
        // printf("Player Disconnected\n");
	} else if (bytes == -1) {                           //Opponent Disconnects
		printf("Terminating: %s\n",strerror(errno));
        // printf("Player Disconnected\n");
	} else {
		printf("Terminating Game\n");                   //The game ends normally
        // printf("Disconnecting Player\n");
	}

    close(players[1].sockID);
    close(players[0].sockID);

    free(players[0].name);
    free(players[1].name);
    initStruct(&players[0]);
    initStruct(&players[1]);
    
    return NULL;
}


int nameCheck(struct playerInfo **players,char* name){
    for(int i=0; i<numOfGames; i++){
        for(int j=0; j<2; j++){
            if(players[i][j].name == NULL){
                continue;
            }
            if( strcmp(players[i][j].name,name) == 0){
                return 1;
            }
        }
    }
    return 0;
}

void freeing(struct playerInfo **players){
    for(int i=0; i<numOfGames+1; i++){
        for(int j=0; j<2; j++){
            free(players[i][j].name);
        }
        free(players[i]);
    }
    free(players);
}


int main(int argc, char **argv){
    sigset_t mask;

    struct sockaddr_storage remote_host;
    socklen_t remote_host_len = sizeof(remote_host);

    int error;
    pthread_t tid;

    char *service = argc == 2 ? argv[1] : "8750";

	install_handlers(&mask);
	
    int listener = open_listener(service, QUEUE_SIZE);
    if (listener < 0) exit(EXIT_FAILURE);
    
    printf("Listening for incoming connections on %s\n", service);

    int numOfPlayers=0;

    struct playerInfo **players = (struct playerInfo** ) calloc(1, sizeof(struct playerInfo *)) ;          //An array to hold the struct arrays for each game. 
    players[numOfGames] = (struct playerInfo*) calloc(2, sizeof(struct playerInfo) );                  //Gaame structs
    initStruct(&players[numOfGames][0]);
    initStruct(&players[numOfGames][1]);


    while (active) {

        // struct playerInfo* players = holder[numOfGames];     //Stores the information for the game. 
    
        
        while(numOfPlayers < 2 && active){
            players[numOfGames][numOfPlayers].sockID = accept(listener, (struct sockaddr *)&remote_host, &remote_host_len);
            if (players[numOfGames][numOfPlayers].sockID < 0) {
                perror("accept");
                continue;
            }
            printf("Player %d has joined\n",numOfPlayers + 1);

            char buf[BUFSIZE + 1];
            int bytes;
            if( (bytes = read(players[numOfGames][numOfPlayers].sockID, buf, BUFSIZE) ) >0){

                //printf("Message Recieved: '%s'\n",buf);
                
                char *start = strchr(buf,'|') + 1;      //Gets the pointer to the first | in the received message
                char *end = strchr(start,'|');          //Gets the pointer to the second | in the received message to get the integer length
                int distance = end - start;
                char fieldLen[distance + 1];
                strncpy(fieldLen,start,distance);
                fieldLen[distance] = '\0';
                int fieldLength = atoi(fieldLen);           //What is the length of the message sent

                if( (bytes - 4 - 1 - distance - 1 - fieldLength) > 0){          //If we don't recieve the entire message
                    write(players[numOfGames][numOfPlayers].sockID,"INVL|18|!Invalid Message!|",strlen("INVL|18|!Invalid Message!|") + 1);
                    printf("Didn't recieve the entire message");
                    close(players[numOfGames][numOfPlayers].sockID);
                    continue;
                }

                if( buf[4] != '|'){     //PLAY|10|Joe Smith|
                    write(players[numOfGames][numOfPlayers].sockID,"INVL|18|!Invalid Message!|",strlen("INVL|18|!Invalid Message!|") + 1);
                    close(players[numOfGames][numOfPlayers].sockID);
                    continue;
                }

                // ____________________________________________________________________________________________________________
                //|                                                UNPACK MESSAGE                                              |
                //|____________________________________________________________________________________________________________|

                char action[5];
                memcpy(action,buf,4);
                action[4] = '\0';

                if( strcmp(action,"PLAY") == 0){        //If our action word is PLAY
                    if(players[numOfGames][numOfPlayers].name == NULL){           //If the player doesn't exist
                        players[numOfGames][numOfPlayers].name = (char*) malloc(fieldLength * sizeof(char));
                        memcpy(players[numOfGames][numOfPlayers].name,end+1,fieldLength-1);
                        players[numOfGames][numOfPlayers].name[fieldLength-1] = '\0';                                 //Save the Player's name

                        //printf("Player name: %s\n",players[numOfGames][numOfPlayers].name);

                        if( nameCheck(players,players[numOfGames][numOfPlayers].name) == 1 ){
                            printf("InvalidName. There is a duplicate\n");
                            write(players[numOfGames][numOfPlayers].sockID,"INVL|25|Already playing the game|",strlen("INVL|25|Already playing the game|" + 1));
                            free(players[numOfGames][numOfPlayers].name);
                            initStruct(&players[numOfGames][numOfPlayers]);
                            close(players[numOfGames][numOfPlayers].sockID);
                            continue;
                        }
                        else if( (numOfPlayers == 1) &&( strcmp(players[numOfGames][numOfPlayers].name,players[numOfGames][0].name) == 0) ){
                            write(players[numOfGames][numOfPlayers].sockID,"INVL|25|Already playing the game|",strlen("INVL|25|Already playing the game|" + 1));
                            free(players[numOfGames][numOfPlayers].name);
                            initStruct(&players[numOfGames][numOfPlayers]);
                            close(players[numOfGames][numOfPlayers].sockID);
                            continue;
                        }

                        if(numOfPlayers == 0){
                            players[numOfGames][numOfPlayers].role = 'X';         //Saves the player's role as X
                        }
                        else{
                            players[numOfGames][numOfPlayers].role = 'O';         //Saves the player's role as X
                        }
            
                        write(players[numOfGames][numOfPlayers].sockID,"WAIT|0|",strlen("WAIT|0|")+1);
                        numOfPlayers++;

                        continue;
                    }
                    else{                               //If the player exists, provide an error
                        printf("The name of the player isn't NULL\n");
                        write(players[numOfGames][numOfPlayers].sockID,"INVL|25|Already playing the game|",strlen("INVL|25|Already playing the game|"));
                        free(players[numOfGames][numOfPlayers].name);
                        initStruct(&players[numOfGames][numOfPlayers]);
                        close(players[numOfGames][numOfPlayers].sockID);
                        continue;
                    }
                }
                else{               //If the command is not PLAY
                    write(players[numOfGames][numOfPlayers].sockID,"INVL|20|NOT A VALID COMMAND|",strlen("INVL|20|NOT A VALID COMMAND|") + 1);
                    close(players[numOfGames][numOfPlayers].sockID);
                    continue;
                }
            }
            else{       //Unable to read
                close(players[numOfGames][numOfPlayers].sockID);
                continue;
            }
        }


        // ____________________________________________________________________________________________________________
        //|                                             Get pthread ready                                              |
        //|____________________________________________________________________________________________________________|



        // temporarily disable signals
        // (the worker thread will inherit this mask, ensuring that SIGINT is
        // only delivered to this thread)
        error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
        if (error != 0) {
        	fprintf(stderr, "sigmask: %s\n", strerror(error));
        	exit(EXIT_FAILURE);
        }
        
        //printf("Gonna create a thread\n");

        if(active){
            error = pthread_create(&tid, NULL, read_data, (void*) players[numOfGames]);
        }
        
        
        if (error != 0) {
        	fprintf(stderr, "pthread_create: %s\n", strerror(error));
        	close(players[numOfGames][0].sockID);
            close(players[numOfGames][1].sockID);
        	free(players[numOfGames][0].name);
            free(players[numOfGames][1].name);
            initStruct(&players[numOfGames][0]);
            initStruct(&players[numOfGames][1]);
        	continue;
        }
        
        // automatically clean up child threads once they terminate
        pthread_detach(tid);
        
        // unblock handled signals
        error = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
        if (error != 0) {
        	fprintf(stderr, "sigmask: %s\n", strerror(error));
        	exit(EXIT_FAILURE);
        }

        numOfGames++;
        numOfPlayers = 0;
        players = realloc(players,(numOfGames+1) * sizeof(struct playerInfo *)) ;          //An array to hold the struct arrays for each game. 
        players[numOfGames] = (struct playerInfo*) malloc(2 * sizeof(struct playerInfo) );
        initStruct(&players[numOfGames][0]);
        initStruct(&players[numOfGames][1]);

    }

    freeing(players);

    puts("Shutting down");
    close(listener);
    
    // returning from main() (or calling exit()) immediately terminates all
    // remaining threads

    // to allow threads to run to completion, we can terminate the primary thread
    // without calling exit() or returning from main:
    //   pthread_exit(NULL);
    // child threads will terminate once they check the value of active, but
    // there is a risk that read() will block indefinitely, preventing the
    // thread (and process) from terminating
    
	// to get a timely shut-down of all threads, including those blocked by
	// read(), we will could maintain a global list of all active thread IDs
	// and use pthread_cancel() or pthread_kill() to wake each one

    return EXIT_SUCCESS;
}
