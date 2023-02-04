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

const unsigned int MAX_PORT_SIZE = 65535;
const unsigned int MIN_PORT_SIZE = 100;

using namespace std;

int main (int argc, char** argv) {

    if (argc != 7) {
        cout << "Must pass in 6 parameters in terminal. " << endl;
        return -1;
    }

    //initializing the port that will be listend to
    int serverPort = atoi(argv[1]);
    if( serverPort < MIN_PORT_SIZE || serverPort > MAX_PORT_SIZE) {
        cerr << "Server port must be between 100 and 65535" << endl;
        return -1;
    }

    //taking in the number of times the loop will execute.
    int repetition = atoi(argv[2]);
    if (repetition < 1) {
        cerr <<"Repetition must be greater than 0" << endl;
        return -1;
    }

    //receiving the nbufs and bufsize and checking if multiplies to
    //1500
    int nbufs = atoi(argv[3]); 
    int bufsize = atoi(argv[4]); 
    if (nbufs * bufsize != 1500) {
        cerr << "values must multiply to 1500" << endl;
        return -1;
    }

    //taking the server IP name from terminal and storing it.
    char* server_ip = argv[5];

    //taking type in and checking if not between 1 and 3
    int type = atoi(argv[6]);
    if ( type < 1 || type > 3) {
        cerr << "value must be 1 2 or 3" << endl;
        return -1;
    }


    //setting hostent to server IP name    
    struct hostent* host = gethostbyname(server_ip);
    sockaddr_in sendSockAddr;
    bzero((char*) &sendSockAddr, sizeof(sendSockAddr));
    sendSockAddr.sin_family = AF_INET;
    sendSockAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(serverPort);
    //creating a socket using TCP
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    int connectStatus = connect(clientSd, (sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
    if(connectStatus < 0) {
        cerr << "failed to connect to the server" << endl;
    }

    //allocating databuf 2D array with buf values
    char databuf[nbufs][bufsize];


    //initializing timne variables for equations
    struct timeval start, lap, end, data_sending_time, round_time;
    int r_sec, r_usecs, s_sec, s_usecs, lapped;
    gettimeofday( &start, NULL );

    //looping repetition times and checking for each write type
    for (int i = 0; i < repetition; i++)
    {
        if (type == 1)
        {
            for ( int j = 0; j < nbufs; j++ )
            {
                write( clientSd, databuf[j], bufsize ); 
            }
        }

        else if (type == 2)
        {
            struct iovec vector[nbufs];
            for ( int j = 0; j < nbufs; j++ ) {
                vector[j].iov_base = databuf[j];
                vector[j].iov_len = bufsize;
            }
            writev( clientSd, vector, nbufs ); 

        }

        else
        {
            write( clientSd, databuf, nbufs * bufsize );
        }

    }

    gettimeofday(&lap, NULL);


    //retrieving recount value for reads in server.
    int recount;
    read(clientSd, &recount, sizeof(recount));

    //equation for seconds and useconds
    gettimeofday(&end, NULL);
    s_sec = lap.tv_sec - start.tv_sec;
    s_usecs = lap.tv_usec - start.tv_usec;
    s_usecs += s_sec * 1000000;
    r_sec = end.tv_sec - start.tv_sec;
    r_usecs = end.tv_usec - start.tv_usec;
    r_usecs += r_sec * 1000000;


    //printing values to console 
    cout << "Test " << type << 
            ": Data_sending_time = " << s_usecs << 
            " usecs, Round_Trip_time = " << r_usecs << 
            " usecs, #reads = " << recount << endl;



    close(clientSd);

}