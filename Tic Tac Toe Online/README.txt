NetID: 
Pranav Angiya Janarthanan
Dhruvita Patel 

For this project, the following files have been included: 
* Makefile used for code compilation
* tts.c server code to run the server
* README.txt that contains information about the code and how it works
No client side code was provided as it not neccesary to include in the submission. A temporary client side program was used for testing purposes but it doesn't send messages in protocall format. As such, a client side was not provided but it is neccesary to connect to the server. When connecting, it is neccesary to connect to the PORT 8750 when no arguments are being passed into the server TTS.C


This project relies on us to utilize multitasking components such as multiprocessing or multithreading along with network communication to set up a Tic Tac Toe Server, which allows external communications from other devices or networks to play an online version of this popular game. For this project, there are some crucial components of the project that must be handled first before we can begin creating our server. 


One of the most fundamental components of this project is to establish a network connection for a server. We need to create a server such that it allows other networks to connect to this server and allow communication between them using sockets. For this project, we have a majority of the server code stored in ttts.c. Using our make file, we will be able to compile and run the following code without any hassle. We will create our application as a server by using the open_listener function. This function is the most crucial part of our project as it allows us to establish streaming sockets between the different network communications. For our projects, we will be utilizing a default PORT number of 8750 for our testing. Having this unique number ensures that we will not accidentally connect to another server if they have the same domain. The function outputs a listener socket which allows us the function to accept network connections to that specific listener socket. We utilize this inside the main function, which will control the majority of the network communications. 


The main information will be stored in the form of a struct which allows us to keep track of important information such as the player's name, their socket ID with the server, and their role as either X or O. 


Getting into the majority of the connection code, we will be using an extremely simple client side code for this project. This client does not have any important or useful code except for establishing a socket connection to the server. To connect to the server, the server must perform the following actions in these steps:
1. If the number of players connected to the server is less than 2, we will proceed to the following step
2. We will establish a connection between the server and the client. But before we can ensure that this client is valid, the client must first provide the action word PLAY along with their name. We will be storing all names in a separate array to ensure there are no duplicate names in the server. 
3. As long as these conditions are met, the server will then rerun the above code and attempt to find another client that the server can connect with. 
4. Once the server has two clients that are valid and can enter into a game, the server will push them into a thread that handles all of the games. 


As we established, for a client to join a game, they must have a unique name, and they must enter the phrase PLAY. If any one of these is not met, the server will automatically end the socket communication with this client and look for other clients that can meet this condition. While a client is waiting for an opponent, it will receive the command WAIT, which instructs the client to wait for another opponent to join. Once the second client joins and is validated, it will also receive the command WAIT till it enters into a game. 

Once the two clients join the game, they will then be able to proceed by both of them receiving the command BEGN along with their role as either X or O. This will be done inside the function read_data, an extremely important function that contains a majority of the code. This function will have a struct PlayerInfo as its argument, which is simply an array of structs that contains all the player information. The players will be assigned a role based on a first-come, first-serve basis. At this point, the two clients will be able to enter in some commands. But before they are able to enter some commands in, some setup is required. We will create the board of the game inside the thread function as a Char Multidimensional array where an empty spot is stored as the char E and an occupied spot will be represented using the players role. 


One important piece of information about the board and how to enter in information when MOVE. In order to move to a desired location, the client will first have to enter the horizontal location of the spot they wish to occupy, followed by the vertical spot. For example, if the user wants to place their piece into the top right corner, they would need to type in MOVE 3,1 to indicate the 3rd and last position on the board horizontally and the 1st row. 


When the game begins, the player with X role will be able to make their move first. They can have one of four commands: MOVE, RSGN, and DRAW. These commands will be read from the buffer and stored inside the variable Action, which stores what action the user wants to perform. Let us start with the simpler of the three, RSGN or Resign. 


RSGN is a command that can be entered without any additional arguments. When a client enters in the command, the client is forfeiting the game and quitting. As a result, the opponent will end up winning the game and receiving the command OVER along with argument W since they win. The client who chooses to resign will receive the argument L since they decided to forfeit and lose. 


DRAW is a command that must be initiated with an S to suggest that the current user wants to propose a peaceful draw of the game. This draw means that the user wants to terminate the game with no winner, and both players will have a draw. The opponent will be able to respond to the following message using either A for accepting the draw or R for rejecting the draw. We will be performing this by first checking if the user entered in S and initiated the call. If so, we will forward that proposal to the opponent by writing the information into the opponent's socket, which we can get from the struct that we passed into the main game function. Once that is done, inside the same draw check, we will read the input from the opponent. We will make sure that the command they have provided is a valid command and can run as intended. Several checks, such as ensuring there are no protocol errors and other errors, will be checked along the way. If there are any errors, a do-while loop will make sure that information is constantly being fetched till we get the desired result. Once that is done, the opponent will send their decision to the client, which can either be a rejection of the proposal or an acceptance of the proposal. If the opponent chooses to reject the proposal, the server will write to the player's socket and notify them that the opponent has rejected the proposal. As a result, the player will then be able to make a move or send another proposal, or withdraw. 


However, if the player decides to MOVE with a horizontal position on the board followed by a vertical position on the board, we will enter the Tic Tac Toe Game Function. 


TIC TAC TOE GAME Function
* Created a 3x3 board and filled each tile with the character E to represent that spot is empty 
* Within game function, there is 5 different return options
   * Return 1->valid input but no winners
   * Return 2-> not valid input
   * Return 3-> player X wins
   * Return 4->player O wins
   * Return 5-> there is no more spots left leading to a draw
* Then scan for user input in a format of (horizontal value,vertical value) with values ranging from 1-3
   * If the value is out of that range, it will fail and ask for new input
* Then there is a subtraction of one to horizontal value and vertical value to match array placement 
* If the spot that the user input has the character E then it will be replaced with the user’s marker
   * Run through all helper functions to check for a win or a draw
* Else it will fail and prompt the user for new input


Helper functions
* Vertical (function name: verti)
   * With the input of the vert and player, the function checks all horizontal values with the user’s input vertice
* Horizontal(function name: horiz)
   * With the input of the horizontal position and player, the function checks all vertical values with the user’s input horizontal 
* Diagonal (function name: dia)
   * Checks the point (2,2) is held by a player first since that is required to get any diagonal win on the board
      * Then checks the combination of (1,1) and (3,3) OR (1,3) and (3,1) is held by the player if so return TRUE else returns FALSE
* Draw (function name: draw)
   * Run through every tile in the board to check if there is no more empty spots (board not equal to ‘E’)


Let's assume that a normal move has been made, and no player has won or drawn. As a result, it will update the board. Now, we will send the message back to both the clients of the move made. Since we have stored empty spots as the character ‘E’, we can create a very simple algorithm that traverses through the entire board and stores the information into a string where the empty spots are stored as ‘.’ and any occupied spots will stored as the respective player’s key. Finally, we will notify the clients of this move by sending MOVD, which is a command used to display the move made. However, if it is found that one of the players made a winning move, the outputs of the game function will indicate which player has won, as explained above. At this point, the winning client will receive an OVER command with an argument indicating that they have won. Additionally, the losing client will receive the same command but an argument indicating that they have lost. Finally, if a draw was issued, we would receive the same command OVER and argument D indicating the draw alongside a positive message about both clients trying hard to achieve their goal of Tic Tac Toe Domination.  


However, let's assume that the client has provided an invalid command, one that isn’t recognized, the server will write into the socket and send an INVL command indicating an error with the command. 


Any time the client makes a valid move that does not result in any finishing move, they will trigger a switch labeled the CurrentPlayer variable which indicates which player is currently making a move. This is what’s used to keep track of players’ turns and any information being read. 


If the client decides to input any hazardous commands, whether it was caused due to a faulty write into the socket, or an extreme amount of invalid arguments, the server will assume that this is a deadly protocol error that was made by the client, resulting in an immediate disconnection between the server and the two clients. This will hopefully incentivize the client not to make any dangerous errors that could jeopardize the game for their opponent. 


Often times if a player disconnects for whatever reason, they will issue a value of 0 for the read-while loop that controls the entire game. This will terminate the entire game completely for both players. This is used as an indication for any weird disconnections.

