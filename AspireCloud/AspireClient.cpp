/*
Brian Kieffer, Nicholas Martin, Harvy Barsoom
Purpose: The purpose of this file is to represent client side. 4 clients can play this game and receive messages from the server. It is team
based game where all four players need to work together and defeat the monster. In this current version of the game a created game will
already be made and wait for 4 users to join in. Each user must ready up before the game will actually start. Users can exit gracefully when they
type in EXIT and it will close the client and end the program. Nicholas focused a lot of his energy on the client side as well as Harvy. We
tried using C# and Unity at first, however we struggled and Unity was being a pain for us. Myself(Brian) was working on the server side. 
*/
#include<iostream>
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>     // gethostbyname
#include <unistd.h>    // read, write, close
#include <strings.h>     // bzero
#include <cstring>      // needed for strlen to work
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <iostream>
#include <random>

std::random_device rd;  //Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<> randomInt(2,5);    // set up the range


using namespace std;
struct player{
    std::string username;
    int energy; //not used yet
    int HP = 8;
    int shields = 0;
    int maxShields = 4;
};
bool checkMess(string mess, player user, int clientSD);

const unsigned int BUF_SIZE = 65535;

int main () {
    player user;
    int server_port = 10743; //Creates the socket
    struct hostent* host = gethostbyname("10.155.176.28"); //Gets host IP Address
    char buffer[1024];

    sockaddr_in sendSock; //Initializes
    bzero((char*) &sendSock, sizeof(sendSock)); //zeros out data structure
    sendSock.sin_family = AF_INET; //Using IPv4?
    sendSock.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list)); //sets to IP address we looked up
            //What does this all mean? ^
    sendSock.sin_port = htons(server_port); //Sets port to connect to (declared above)

    int clientSd = socket(AF_INET, SOCK_STREAM, 0); //Creates new socket for TCP/IP

    int connectStatus = connect(clientSd, (sockaddr*)&sendSock, sizeof(sendSock)); //if connects, returns 0
    
    if (connectStatus < 0){
        cout << "Connection Error" <<endl;
    }
    string username = "";
    cout << "Please enter your username" << endl;
    cin >> username;
    user.username = username;
    write(clientSd, user.username.c_str(), 20); //sends command line argument
    
    int input = 0;
    cout << "waiting for message from server" << endl;
    read(clientSd, buffer, 1024);
    cout << buffer << endl;
    
        cout << "please enter 1 to ready up" << endl;
        cin >> input;
        if(input == 1){
            write(clientSd, "READY", 5); //sends command line argument
        }
        string message = "";
        string work = "";
    while(true){
        bzero(buffer, 1024);
        read(clientSd, buffer, 1024);
        cout << buffer << endl;
        work = buffer;
        for(int i = 0; i < 1000; i++){}
        if(checkMess(work, user, clientSd)){
            while(true){
                // cout << "enter a message or enter EXIT to disconnect" << endl;
                // cin >> message;
                // if(message == "EXIT"){
                //     close(clientSd);
                //     break;
                // }
                // char mess[message.length() + 1];
                // strcpy(mess, message.c_str());
                // write(clientSd, mess, sizeof(mess));

                // buffer[0] = 0;
                // read(clientSd, buffer, 1024);
                // cout << buffer << endl;
                int attack = 0;
                cout << "Enter 1 to attack or 2 to defend!"<<endl;
                cin >> message;
                char mess[5];
                if(message == "1") {
                    mess[0] = 'A';
                    attack = randomInt(gen);
                    cout<<"You attack for "<<attack<<" damage!"<<endl;
                }
                if(message == "2"){ 
                    mess[0] = 'D';
                    user.shields += 2;
                    if(user.shields > user.maxShields) user.shields = user.maxShields;
                    cout<<"You defend and have "<<user.shields<<" shields!";
                }
                if(message != "1" || message != "2"){cout<<"Selection Invalid!"<<endl;}
                else{
                    char mess[5];
                    mess[2] = (char)attack;
                    mess[4] = (char)user.shields;
                    write(clientSd, mess, sizeof(mess));
                    buffer[0] = 0;buffer[1]=0;buffer[2] = 0;buffer[3]=0;buffer[4]=0;
                    read(clientSd, buffer, 1024);
                    cout<< "The enemy has "<<buffer[1]<<buffer[2]<<" Health and "<<buffer[3]<<buffer[4]<<" Shields!"<<endl;
                    break;
                }
            }
        }
        if(message == "EXIT"){
            break;
        }
    }
    return 0;

};
/*
Function to check the messages from the server. They are hard-coded messages we expect such as STARTYou go and Your Turn.
If these messages are found from the server it gives the user an ability to write to the server. Our intentions with this is to 
allow the user to write a message to the server with their move choice. 
*/
bool checkMess(string mess, player user, int clientSD){
    string temp = "";
    for(int i = 0; i < mess.length(); i++){
        if(temp == "STARTYou go" || temp == "Your turn"){
            return true;
        }
        else{
            if (mess[0] == 'A'){   
                int damage = (int)mess[1]*10+(int)mess[2];
                cout<<"The creature attacks for "<<damage<<" damage!";
                if (user.shields >= damage){
                    user.shields -= damage;
                } else {
                    damage -= user.shields;
                    user.shields = 0;
                    user.HP -= damage;
                }
                cout<<"You have "<<user.HP<<" Health and "<<user.shields<<" Shields!"<<endl;
                string response = "R0"+user.HP;
                response += "0"+user.shields;                    
                char mess[6];
                strcpy(mess, response.c_str());
                write(clientSD, mess, sizeof(mess));
            } else if (mess[0] == 'D'){
                cout<<"The creature defends!";
                string response = "R0"+user.HP;
                response += "0"+user.shields;                    
                char mess[6];
                strcpy(mess, response.c_str());
                write(clientSD, mess, sizeof(mess));
            } else if (mess[0] == 'R'){
                cout<<"After recieving an attack, the monster has "<<mess[1]<<mess[2]<<" Health and "<<mess[3]<<mess[4]<<" Shields!";
            }
        }
    }
    return false;
}