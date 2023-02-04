#include <iostream>
#include <sys/types.h> // Socket, bind
#include <sys/socket.h> // socket, bind, listen, inet_ntoa
#include <netinet/in.h> // htonl, htons, inet_ntoa
#include <arpa/inet.h> // inet_ntoa
#include <netdb.h> // gethostbyname
#include <unistd.h> // read, write, close
#include <strings.h> // bzero
#include <netinet/tcp.h> // SO_REUSEADDR
#include <sys/uio.h> // writev
#include <string>
#include <sys/time.h>
#include <cstring>
#include <fstream>

const unsigned int MAX_PORT_SIZE = 65535;
const unsigned int MIN_PORT_SIZE = 100;
const unsigned int BUFSIZE = 100;

using namespace std;

bool isRequestValid(string response) {
    return
        response.find("301 Moved Permanently") != string::npos ||
        response.find("400 Bad Request") != string::npos ||
        response.find("401 Unauthorized") != string::npos ||
        response.find("403 Forbidden") != string::npos ||
        response.find("404 Not Found") != string::npos || 
        response.find("408 Request Timeout") != string::npos;
}

int main (int argc, char** argv) {

    if (argc != 4) {
        cout << "Must pass in 3 parameters in terminal. " << endl;
        return -1;
    }

    // string for server
    string server = argv[1];

    // string for file name
    string file = argv[2];


    if (file.substr(0, 7) == "http://")
        file.erase(0, 7);

    if (file.substr(0, 8) == "https://")
        file.erase(0, 8);
    

    //initializing the port that will be listend to
    int serverPort = atoi(argv[3]);
    if (serverPort != 80 && serverPort != 8080) {
        cerr << "Server port must be 80 or 8080" << endl;
        return -1;
    }


    //setting hostent to server IP name    
    struct hostent* host = gethostbyname(server.c_str());
    sockaddr_in sendSockAddr;
    bzero((char*) &sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(serverPort);
    //creating a socket using TCP
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    int connectStatus = connect(clientSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
    if (connectStatus < 0) {
        cerr << "failed to connect to the server" << endl;
        return -1;
    }

    cout << "Passed validation checks" << endl;

    string request = string("GET " + file + " HTTP/1.1\r\n" + 
                    "Host: " + server + "\r\n" + "\r\n");   //create the http request

    cout << "Sending Request: \n" << request << endl;
    int sendResult = write(clientSd, request.c_str(), strlen(request.c_str())); //send info to server

    string response;
    int location = 0;
    bool errorRecieved = false;
    bool responseComplete = false;

    char buffer[100];
    char* readBuffer = buffer;

    int n = 0;
    while (!responseComplete || !errorRecieved) {
        int n = read(clientSd, readBuffer, 5);
        readBuffer[n] = '\0';
        response += readBuffer;
            //find the size of the html body
            if(response.find("Content-Length:") != string::npos)
            {
                location = response.find("Content-Length:");
                location += 15;
                atoi(response.substr(location , response.length()).c_str());
            }
            // If request is not valid
            if(isRequestValid(response))
            {
                errorRecieved = true;
            }
            // If the request is complete
            if(response.find("\r\n\r\n") != string::npos) {
                responseComplete = true;
            }
    }

    ofstream out;
    out.open("extra");
    out << response;

    close(clientSd);
}
