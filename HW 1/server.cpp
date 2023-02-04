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


using namespace std;

const unsigned int BUF_SIZE = 1500;
const unsigned int MAX_PORT_SIZE = 65535;
const unsigned int MIN_PORT_SIZE = 100;


//thread data struct for pthread create
struct thread_data {
   int sd;
   int repetition;
   char* databuf;
};

//thread server method for creating thread
void *thread_server(void *ptr) {
    //setting the data to ptr as thread_data pointer type
    thread_data* data = (struct thread_data*) ptr;
    char databuf[BUF_SIZE];
    data->databuf = databuf;
    //initializing time variables.
    struct timeval start, end;
    int count = 0;

    gettimeofday( &start, NULL );
    //looping through repetition times to read from client and incrementing count.
    for (int i = 0; i < data->repetition; i++) {
        for(int nread = 0; (nread += read(data->sd, data->databuf, BUF_SIZE - nread)) < BUF_SIZE; ++count);
        count++;
    }
    //writing to client 
    write(data->sd, &count, sizeof(count));	
    //ending timer.
    gettimeofday(&end, NULL);

    //calculating time to recieve data.
    int data_receiving_time = (end.tv_sec - start.tv_sec) * 1000000 + 
                              (end.tv_usec - start.tv_usec);

    cout << "data receiving time = " << data_receiving_time << " usec" <<endl;

    close(data->sd);
    free(ptr);
    return NULL; 

}


int main (int argc, char** argv) {


    //checking if args are at least 2 for variables.
    if (argc != 3) {
        cerr << "Must pass in 2 parameters in terminal. " << endl;
        return -1;
    }

    //checking serverPort for max and min
    int serverPort = atoi(argv[1]);
    if (serverPort < MIN_PORT_SIZE || serverPort > MAX_PORT_SIZE) {
        cerr << "Server port must be between 100 and 65535" << endl;
        return -1;
    }

    //checking for non zero value for repetiitons.
    int repetition = atoi(argv[2]); 
    if (repetition < 1) {
        cerr << "repetition must be greater than 0" << endl;
        return -1;
    }

    //allocating databuf array and accepting the sockets to connect to client
    sockaddr_in acceptSock;
    bzero((char*) &acceptSock, sizeof(acceptSock));  // zero out the data structure
    acceptSock.sin_family = AF_INET;   // using IP
    acceptSock.sin_addr.s_addr = htonl(INADDR_ANY); // listen on any address this computer has
    acceptSock.sin_port = htons(serverPort);  // set the port to listen on
    int serverSd = socket(AF_INET, SOCK_STREAM, 0); // creates a new socket for IP using TCP
    const int on = 1;
    setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(int));  // this lets us reuse the socket without waiting for hte OS to recycle it

    // Bind the socket
    bind(serverSd, (sockaddr*) &acceptSock, sizeof(acceptSock));  // bind the socket using the parameters we set earlier
    
    // Listen on the socket
    int n = 5;
    listen(serverSd, n);  // listen on the socket and allow up to n connections to wait.

    // Accept the connection as a new socket
    sockaddr_in newsock;   // place to store parameters for the new connection
    socklen_t newsockSize = sizeof(newsock);


    //accepting the socket and creating a new thread to use the socket.
    while(1) {

        int newSd = accept(serverSd, (sockaddr *)&newsock, &newsockSize);  // grabs the new connection and assigns it a temporary socket
        //creating a new thread and calling pthread_creates
        thread_data *data = new thread_data;
        data->repetition = repetition;
        data->sd = newSd;
        pthread_t new_thread;
        int iret1 = pthread_create(&new_thread, NULL, thread_server, (void*) data );

        //error check for thread.
        if(iret1 < 0) {
            cerr << "Error in creating thread" << endl;
            return -1;
        }

        pthread_join(new_thread, NULL);

        close(newSd);

    }

    return 0;

}
