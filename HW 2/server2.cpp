#include<iostream>
#include <pthread.h>
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>     // gethostbyname
#include <unistd.h>    // read, write, close
#include <strings.h>     // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <sys/time.h>
#include <fstream>
#define SERVERPORT 8080

using namespace std;



//thread data struct for pthread create
struct thread_data {
   int sd;
   int threadId;
   char* databuf;
};

//thread server method for creating thread
void* thread_server(void *ptr) {
    
    thread_data* data =  (struct thread_data*) ptr;
    string fileName = "";
    string hostName = "";
    string headers = "";
    string headerStatus= "";
    int location = 0;
    int length = 0;
    char buffer[100];
    char* buf = buffer;
    bool finished = false;
    cout << "Begin operator" << endl;
    while(!finished)
    {
        int n = read(data->sd, buf, 5);
        buf[n] = '\0';
        headers += buf;
         if(headers.find("\r\n\r\n") != string::npos) {
            finished = true;
         }
    }
    //check if the header request is properly formatted
    if(headers.find("GET") != string::npos && headers.find("HTTP/1.1") != string::npos && headers.find("Host:") != string::npos){
            location = headers.find("HTTP/1.1");
            location -= 5; //Offset to find the location of the file (starting pos)
            fileName = headers.substr(4, location);
            location = headers.find("Host:");
            location += 6;
            hostName = headers.substr(location, headers.length());
    } 
    else
    {
        headerStatus = "HTTP/1.1 400 Bad Request\r\n";
    }
    time_t timetoday;
    time(&timetoday);
    string cur_time = asctime(localtime(&timetoday));
    cur_time += "\r\n\r\n";
    FILE* file;
    if(headerStatus == "")
    {
        cout << "Opening file " << endl;
        if(fopen(fileName.c_str(), "r") == NULL)
        {
            cout << "Unable to open file!" << endl;

            if(EACCES == true || fileName == "secret.txt")
            {
                headerStatus = "HTTP/1.1 401 Unauthorized\r\n";
            }
            else if(fileName.substr(1,fileName.length()).find("/") != string::npos)
            {
                headerStatus = "HTTP/1.1 403 Forbidden\r\n";
            }
            else if(fileName.substr(0,2) == "..")
            {
                headerStatus = "HTTP/1.1 403 Forbidden\r\n";
            }
           
            else
            {
                headerStatus = "HTTP/1.1 404 Not Found\r\n";
            }
            
        }
        else    //The header request was good and file was found
        {
            file = fopen(fileName.c_str(), "r");
            fseek(file, 0, SEEK_END);
            length = ftell(file);
   
        }
    }
    string response = headerStatus + cur_time + "Content Length: " + to_string(length) + "\r\nContent-Type: text-plain\r\n";
    cout << response << endl;
    write(data->sd, &response[0], response.size());
    char rbuffer[length];
    fgets(rbuffer, length + 1, file);
    write(data->sd, rbuffer, sizeof(buffer));
    close(data->sd);
}


int main (int argc, char** argv) {


    //checking if args are at least 2 for variables.
    if (argc != 2) {
        cerr << "No arguments expected " << endl;
        return -1;
    }


    //allocating databuf array and accepting the sockets to connect to client
    sockaddr_in acceptSock;
    bzero((char*) &acceptSock, sizeof(acceptSock));  // zero out the data structure
    acceptSock.sin_family = AF_INET;   // using IP
    acceptSock.sin_addr.s_addr = htonl(INADDR_ANY); // listen on any address this computer has
    acceptSock.sin_port = htons(SERVERPORT);  // set the port to listen on
    int serverSd = socket(AF_INET, SOCK_STREAM, 0); // creates a new socket for IP using TCP
    const int on = 1;
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int));  // this lets us reuse the socket without waiting for hte OS to recycle it

    // Bind the socket
    bind(serverSd, (sockaddr*) &acceptSock, sizeof(acceptSock));  // bind the socket using the parameters we set earlier
    
    // Listen on the socket
    int n = 5;
    listen(serverSd, n);  // listen on the socket and allow up to n connections to wait.

    sockaddr_in newsock;   // place to store parameters for the new connection
    socklen_t newsockSize = sizeof(newsock);
    int count = 0;
    while(true)
    {
        int newSd = accept(serverSd, (sockaddr *)&newsock, &newsockSize);  // grabs the new connection and assigns it a temporary socket
        if(newSd <= 0 )
        {
            cerr << "Unable to connect to client!" << endl;
            continue;
        }
        thread_data *data = new thread_data;    //create the struct
        data->sd = newSd;    
        data->threadId = count;
        pthread_t thread;
        count++;
        int result = pthread_create(&thread, nullptr, thread_server, (void*) data);
        cout << "Creating thread: " << to_string(count) << endl;
        if ( result != 0 ){
            cout << "Unable to create thread. Trying again" << endl;
            continue;
        }
    }

    // Getting the response 

    return 0;

}

