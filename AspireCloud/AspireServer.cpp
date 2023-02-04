/*
Brian Kieffer, Nicholas Martin, Harvy Barsoom
The purpose of this file to act as the server. Clients/players will connect and await to receive messages from the server. 
The server will generate player struct for each connection and store information such as their username. 
The server awaits connections, once a connection is recognized it stores its socket descriptor and then awaits passively for more 
connections or messages from connections. When a player connects and gives its username, it expects the next message to be ready. 
Once all players are ready then it starts the game and goes into generating turns for the users and then the combat stage. 
*/
#include <stdio.h> 
#include <string.h>   //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>   //close 
#include <arpa/inet.h>    //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <string>
#include <iostream>
#include <vector>
#include <random>
#include <cstring>
     
#define TRUE   1 
#define FALSE  0 
#define PORT 10743

using namespace std;
//struct for users
struct player{
    int socket;
    bool ready = false;
    string username;
    int turnOrder = -10;
    int energy;
    int HP;
};
//struct to generate monsters
struct monster{
    int health = 20;
    int armor = 1;
    int shield = 0;
    int attack = 1;
    int block = 2;
    int powerAttack = 3;
};

monster BrentLagesse;//This is just a joke don't give me an F :(

//Methods
void welcomeToAspire(player clientPlayers[], int numOfClients);
bool checkReady(string message);
string turnMess(player client, string mess);
int monstersMove();
string monsterMess(int actionMove);
void attackingStage(player players[], int numberOfClients);
void sendPlayerList(player players[], int numberOfClients);

int main(int argc , char *argv[])  
{   
    int numOfConnected = 0;
    int max_clients = atoi(argv[1]);
    while(max_clients < 2){
        if(max_clients < 2){//The game only works with atleast 2 players
            cout << "Must have atleast two users to play this game." << endl;
            cout << "Please reenter an integer for the number of players." << endl;
            cin >> max_clients;
        }
        else{
            max_clients = atoi(argv[1]);
        }
    }
    int opt = TRUE;  
    int master_socket , addrlen , new_socket , activity, i , valread , sd;  
    int max_sd;  
    player client_socket[4];
    struct sockaddr_in address; 
         
    char buffer[4096];  //data buffer of 1K 
         
    //set of socket descriptors 
    fd_set readfds;  
         
    //a message 
    const char *message = "Welcome to the lobby for Aspire";  
     
    //initialise all client_socket[] to 0 so not checked 
    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i].socket = 0;  
    }  
         
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
     
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
     
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
         
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
         
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
         
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  
         
    while(TRUE)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
     
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
             
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i].socket;  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
     
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
             
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
             
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs
                  (address.sin_port));  
           
            char inMessage[20];
            read(new_socket,inMessage,20);
                 
            //add new socket to array of sockets and stores their username
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i].socket == 0 )  
                {  
                    numOfConnected++;
                    client_socket[i].socket = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                    client_socket[i].username = inMessage;
                    cout << "This is the client's username " << client_socket[i].username << endl;

                    sendPlayerList(client_socket, numOfConnected);
                    break;  
                }  
            } 
        }  
             
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = client_socket[i].socket;  
                 
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buffer, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);  
                        numOfConnected--;
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i].socket = 0;  
                }  
                    
                //When the server receives a message it checks to see if it is a READY message. If so then it turns the .ready to true 
                else 
                {  
                    string confirm = buffer;
                    if(checkReady(buffer)){
                        if(client_socket[i].ready == true){
                        }
                        else{
                            client_socket[i].ready = true;
                        }                        
                    }
                    int needsReady = max_clients;
                    //For loop to see if all players are ready
                    for(int i = 0; i < max_clients; i++){
                        if(client_socket[i].ready == true){
                            needsReady--;
                            continue;
                        }
                        else{
                            const char *error = "we are still waiting for some players to ready up";
                            //write(sd, error, 51);
                            cout << "sent Information to client" << endl;
                            break;
                        }
                    }
                    //When all players are ready then it enters into the game lobby where the game actually starts.
                    cout << "Sending list of clients who are ready." << endl;
                    sendPlayerList(client_socket, numOfConnected);
                    if(needsReady == 0){
                        cout << "Entering game" << endl;
                        for(int i = 0; i < max_clients; i++){
                            write(client_socket[i].socket, "START", 5);
                        }
                        welcomeToAspire(client_socket, max_clients);
                    }
                    else{
                        cout << "Still waiting for players to ready." << endl;
                    }
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    //buffer[valread] = '\0';  
                    //send(sd , buffer , strlen(buffer) , 0 );  
                }  
            }  
        }  
    }  
         
    return 0;  
};
/*
In this function the game actually begins and starts generating the turn order randomly. It then sends a message to all clients 
which turn order they have. Once that is complete we then enter the attacking stage of the game. 
*/
void welcomeToAspire(player clientPlayers[], int numOfClients){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<>randomInt(1,numOfClients);

    int sd;
    char buffer[1024];
    
    for(int x = 0; x < numOfClients; x++){
        int turn = randomInt(gen);
        clientPlayers[x].turnOrder = turn;
        for(int j = 0; j < numOfClients;){
            if(j == x){
                j++;
            }
            else if(clientPlayers[x].turnOrder != clientPlayers[j].turnOrder){
                j++;
            }
            else{
                turn = randomInt(gen);
                clientPlayers[x].turnOrder = turn;
                j = 0;
            }
        }
        cout << clientPlayers[x].username << " turn is " << clientPlayers[x].turnOrder << endl;
    }
        
    for(int i = 0; i < numOfClients; i++){
        string turnMessage = "You go";
        turnMessage = turnMess(clientPlayers[i], turnMessage);
        char turnNotif[turnMessage.length() + 1];
        strcpy(turnNotif, turnMessage.c_str());
        sd = clientPlayers[i].socket;
        write(sd, turnNotif, sizeof(turnNotif));
    }
    
    cout << "let the attacking begin" << endl;
    attackingStage(clientPlayers, numOfClients);
};
/*
In this function the users are now going to use their turns to attack. It begins with asking the user to send their move. 
Depending on their move the monster will either lose health, or it will inform the server that the user gained shield for its move. 
*/
void attackingStage(player players[], int numberOfClients){
    int sd;
    char buffer[1024];
    cout << "FIGHT!" << endl;
    int responses = numberOfClients;
    while(BrentLagesse.health >= 0){
        while(responses != 0){
            cout << "waiting for a response..." << endl;
            for(int i = 0; i < numberOfClients; i++){
                read(players[i].socket, buffer, 1024); 
                cout << buffer << endl;
                responses--;
            }
        }
        cout << "Monster attacks now" << endl;
        
        int move = monstersMove();
        /*
        if(move == 2 || move == 4){
            buffer[0] = '0';
        }
        else{
            if(move == 1){
                buffer[0] = '1';
            }
            else if(move == 3){
                buffer[0] = '3';
            }
        }
        for(int y = 0; y < numberOfClients; y++){
            sd = players[y].socket;
            write(sd, buffer, sizeof(buffer));
        }
        int input = 0;
        cin >> input;            
        */
        string monsterMessage = monsterMess(move);
        cout << monsterMessage << endl;
        char monstMess[monsterMessage.length() + 1];
        strcpy(monstMess, monsterMessage.c_str());
        for(int i = 0; i < numberOfClients; i++){
            sd = players[i].socket;
            write(sd, monstMess, sizeof(monstMess));
        }
        
        BrentLagesse.health--;
        break;
    }
};
/*
A function to see if the message read from the client is their READY up message. If so it returns true. 
*/
bool checkReady(string message){
    string ready = "READY";
    for(int i = 0; i < 5; i++){
       if(message[i] == ready[i]){
           continue;
       }
       else{
           cout << "uh oh " << message[i] << " and " << ready[i] << endl;
           return false;
       }
   }
   return true;
};
/*
A function that easily generates the turn message
*/
string turnMess(player client, string mess){
    switch(client.turnOrder){
        case 1: mess += " first.";
        break;
        case 2: mess += " second.";
        break;
        case 3: mess += " third.";
        break;
        case 4: mess += " fourth.";
        break;
        default: mess = "Uh oh there seems to be an error.";
        break;
    }
    return mess;
};
/*
Function that will randomly generate the monsters move. There is a 1/6 probability that the monster will use a power attack, 2/6 chance
of using regular attack, and 3/6 chance of putting up its shield. 
*/
int monstersMove(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<>randomInt(1,6);

    int move = randomInt(gen);
    if(move == 1){
        return 3;
    }
    else if(move > 1 && move < 5){
        if(BrentLagesse.shield == 2){
            return 4;
        }
        BrentLagesse.shield = 2;
        return 2;
    }
    else if(move > 4 && move < 7){
        return 1;
    }
    return 0;
};
/*
Function that will easily generate the message for what the monster chose to use for its move. 
*/
string monsterMess(int actionMove){
    string mess = "";
    switch(actionMove){
        case 1: mess += "The monster chooses attack for 1 damage. All players lose 1 health.";
        break;
        case 2: mess += "The monster chooses shields for 2";
        break;
        case 3: mess += "The monster chooses super attack for 3 damage. All players lose 3 health.";
        break;
        case 4: mess += "The monster choose shield but already has full shield";
        break;
        default: mess = "Uh oh there seems to be an error.";
        break;
    }
    return mess;
};
/*
Function that will generate the list of all players that are connected and identifying which users are ready(1) and which are not(0)
*/
void sendPlayerList(player players[], int numberOfClients){
    string list = "";
    for(int i = 0; i < numberOfClients; i++){
        list += players[i].username;
        if(players[i].ready == false){
            list += ",0,";
        }
        else if(players[i].ready == true){
            list += ",1,";
        }
    }
    char listMess[list.length() + 1];
    strcpy(listMess, list.c_str());
    cout << listMess << endl;
    for(int i = 0; i < numberOfClients; i++){
        write(players[i].socket, listMess, sizeof(listMess));
    }
};