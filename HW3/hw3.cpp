#include <iostream>
#include "UdpSocket.h"
#include "Timer.h"
using namespace std;

const int PORT = 5666;       // my UDP port (with student ID)
const int MAX = 20000;        // times of message transfer
const int windowSize = 30;       // maximum window size
const bool check = true;    //use check mode for more information during run
const int TIMEOUT = 1500;     //Timeout specified us

// client packet sending functions
void ClientUnreliable(UdpSocket &sock, int max, int message[]);
int ClientStopWait(UdpSocket &sock, int max, int message[]);
int ClientSlidingWindow(UdpSocket &sock, int max, int message[],int windowSize);

// server packet receiving functions
void ServerUnreliable(UdpSocket &sock, int max, int message[]);
void ServerReliable(UdpSocket &sock, int max, int message[]);
void ServerEarlyRetrans(UdpSocket &sock, int max, int message[],int windowSize );
void serverEarlyRetransCase4(UdpSocket &sock, const int max, int *message, int windowSize, int dropRate);


enum myPartType {CLIENT, SERVER} myPart;

//Main process to run testing functions
int main( int argc, char *argv[] )
{
    int message[MSGSIZE/4]; 	  // prepare a 1460-byte message: 1460/4 = 365 ints;

    // Parse arguments
    if (argc == 1)
    {
        myPart = SERVER;
    }
    else if (argc == 2)
    {
        myPart = CLIENT;
    }
    else
    {
        cerr << "usage: " << argv[0] << " [serverIpName]" << endl;
        return -1;
    }

    // Set up communication
    // Use different initial ports for client server to allow same box testing
    UdpSocket sock( PORT + myPart );
    if (myPart == CLIENT)
    {
        if (! sock.setDestAddress(argv[1], PORT + SERVER))
        {
            cerr << "cannot find the destination IP name: " << argv[1] << endl;
            return -1;
        }
    }

    int testNumber;
    cerr << "Choose a testcase" << endl;
    cerr << "   1: unreliable test" << endl;
    cerr << "   2: stop-and-wait test" << endl;
    cerr << "   3: sliding window" << endl;
    cerr << "   4: sliding window with errors " << endl;
    cerr << "--> ";
    cin >> testNumber;

    if (myPart == CLIENT)
    {
        Timer timer;
        int retransmissions = 0;

        switch(testNumber)
        {
        case 1:
            timer.Start();
            ClientUnreliable(sock, MAX, message);
            cout << "Elasped time = ";
            cout << timer.End( ) << endl;
            break;
        case 2:
            timer.Start();
            retransmissions = ClientStopWait(sock, MAX, message);
            cout << "Elasped time = ";
            cout << timer.End( ) << endl;
            cout << "retransmissions = " << retransmissions << endl;
            break;
        case 3:
            for (int windowSize = 1; windowSize <= windowSize; windowSize++ )
            {
	        timer.Start( );
	        retransmissions = ClientSlidingWindow(sock, MAX, message, windowSize);
	        cout << "Window size = ";
	        cout << windowSize << " ";
	        cout << "Elasped time = ";
	        cout << timer.End( ) << endl;
	        cout << "retransmissions = " << retransmissions << endl;
            }
            break;

        case 4:
                for (int windowSize = 1; windowSize <= 30; windowSize = windowSize + 29) {
                    for (int dropRate = 0; dropRate <= 10; dropRate++) {
                        timer.Start();                                        // start timer
                        retransmissions =
                                ClientSlidingWindow(sock, MAX, message, windowSize); // actual test
                        cerr << "drop rate = ";
                        cout << dropRate << " ";
                        cerr << "Window size = ";                              // lap timer
                        cout << windowSize << " ";
                        cerr << "Elasped time = ";
                        cout << timer.Lap() << endl;
                        cerr << "retransmissions = " << retransmissions << endl;
                    }
                }
                break;    
        default:
            cerr << "no such test case" << endl;
            break;
        }
    }
    if (myPart == SERVER)
    {
        switch(testNumber)
        {
            case 1:
                ServerUnreliable(sock, MAX, message);
                break;
            case 2:
                ServerReliable(sock, MAX, message);
                break;
            case 3:
                for (int windowSize = 1; windowSize <= windowSize; windowSize++)
                {
	            ServerEarlyRetrans( sock, MAX, message, windowSize );
                }
                break;
            case 4:
                  for (int windowSize = 1; windowSize <= 30; windowSize = windowSize + 29) {
                    for (int dropRate = 0; dropRate <= 10; dropRate++) {
                        serverEarlyRetransCase4(sock, MAX, message, windowSize, dropRate);
                    }
                }
            default:
                cerr << "no such test case" << endl;
                break;
        }

        // The server should make sure that the last ack has been delivered to client.

        if (testNumber != 1)
        {
            if (check)
            {
                cerr << "server ending..." << endl;
            }
            for ( int i = 0; i < 10; i++ )
            {
                sleep( 1 );
                int ack = MAX - 1;
                sock.ackTo( (char *)&ack, sizeof( ack ) );
            }
        }
    }
    cout << "finished" << endl;
    return 0;
}

// Test 1 Client
void ClientUnreliable(UdpSocket &sock, int max, int message[])
{
    // transfer message[] max times; message contains is number i
    for ( int i = 0; i < max; i++ )
    {
        message[0] = i;
        sock.sendTo( ( char * )message, MSGSIZE );
        if (check)
        {
            cerr << "message = " << message[0] << endl;
        }
    }
    cout << max << " messages sent." << endl;
}

// Test1 Server
void ServerUnreliable(UdpSocket &sock, int max, int message[])
{
    // receive message[] max times and do not send ack
    for (int i = 0; i < max; i++)
    {
        sock.recvFrom( ( char * ) message, MSGSIZE );
        if (check)
        {
            cerr << message[0] << endl;
        }
    }
    cout << max << " messages received" << endl;
}

/*
Stop and Wait algorithm where client will stop and wait until the server
sends a matching i number (ordered frame) before sending an ACK for the
frame.
Looks for a timeout period of 1500 microseconds before sending again.
*/
int ClientStopWait(UdpSocket &sock, int max, int message[])
{
   int retransmitted = 0;
   int ack = 0;
   Timer timer;
   // Loop until all expected data is sent based on i number
   for (int i = 0; i < max;)
   {
       // Set message[0] to i number
       message[0] = i;
       // Send a message
       sock.sendTo((char *)message, MSGSIZE);

       // Start the timer to receive ACK
       timer.Start();

       // If there is not data to retrieve then check timeout
       while (sock.pollRecvFrom() <= 0)
       {
        // If a timeout occurs, resend data and reset ACK timer
         if (timer.End() > TIMEOUT)
         {
             sock.sendTo((char *)message, MSGSIZE);
             timer.Start();
             // Add to retransmitted time
             retransmitted++;
         }
       }
       // At this point an ACK has been received
       sock.recvFrom((char *)&ack, sizeof(ack));
       // Check if the is are equal, if so increment it
       // Technically this check isn't needed as server orders frames
       if(ack == i)
       {
         i++;
       }
   }
   // Return the number of retransmissions in sending
   return retransmitted;
}


/*
Reliable server where it will order the i number of frames before it's
sent off to the client. Used for Stop and Wait algorithm on client side.
*/
void ServerReliable(UdpSocket &sock, int max, int message[])
{
  int ack = 0;
  // receive message[] max times
  for (int i = 0; i < max;)
  {
    // Do-while is needed as an initial check is needed
    do
    {
      // Receive a message
        sock.recvFrom( (char *)&ack, sizeof(ack));
        if(ack == i)
        {
          // If this is the expected message, send i number back
          sock.ackTo((char *)&i, sizeof(i));
          //Increment i
          i++;
          //Exit while loop
          break;
        }
        //If it doesn't equal, go back to to the top loop
        else
        {
          //Exit while loop
          break;
        }
    }
    //While the message is not the same as the i number
    while(ack != i);
  }
}

/*
Client side for Sliding Window, which calculates the size of the sliding window
as well as the number of retransmissions based on cumulative ACK is.
Looks for a timeout period of 1500 microseconds before retransmissions.
*/
int ClientSlidingWindow(UdpSocket &sock, int max, int message[], int windowSize)
{
  int retransmissions = 0;    // Number of retransmissions
  int ack = 0;            // Ack receives
  int ackSeq = 0;         // Cumulative ack i expected to receive
  Timer timer;            //Timer object to time packet sent and received

  for (int i = 0; i < max || ackSeq < max;)
  {
    // Within the sliding window
    if ( ackSeq + windowSize > i && i < max )
    {
      message[0] = i;             // message[0] has a i number
      sock.sendTo((char *)message, MSGSIZE);
      timer.Start();
      if (sock.pollRecvFrom() > 0)
      {
        sock.recvFrom((char *)&ack, sizeof(ack));
        //If the server message seq = ackSeq
        if(ack == ackSeq)
        {
          ackSeq++;
        }
      }
      //Increment i regardless
      i++;
    }
    //If this larger than the sliding window
    else
    {
      //If the cumulative ack is equal to or greater than max, exit loop
      if (ackSeq >= max)
          break;
      //Continually poll to see what data there is
      //Resend frame as TIMEOUT period has been exceeded
      if(timer.End() > TIMEOUT)
      {
        //Restart timer for resent message
        timer.Start();
        sock.sendTo((char *)message, MSGSIZE);
        //Increase retransmit
        retransmissions++;
      }
      //If the time received is within the valid TIMEOUT range
      while(timer.End() <= TIMEOUT && sock.pollRecvFrom())
      {
        //Receive ACK
        sock.recvFrom((char *)&ack, sizeof(ack));
        //Check the received  message is >= ackSeq
        if(ack >= ackSeq)
        {
          //Updated Cumulative ACK
          ackSeq = (ack + 1);
          //Exit loop as Cumulative ACK has been updated
          break;
        }
        //Resend as something is wrong with ACK
        else
        {
          //Retime resend
          timer.Start();
          sock.sendTo((char *)message, MSGSIZE);
          //Increase retransmit
          retransmissions++;
        }
      }
    }
  }
  return retransmissions;
}

/*
Server side for Sliding Window where it retransmissions cumulative ACKs throughout
the process.
*/
void ServerEarlyRetrans(UdpSocket &sock, int max, int message[],int windowSize )
{
    int ack = 0;                     // an ack message
    bool array[max];                 // indicates the arrival of message[i]
    int i = 0;							   //  i # of message expected
    for (int j = 0; j < max; j++)
    {
       array[j] = false;  		 		 // no message has arrived yet
    }
    for (i = 0; i < max;)
    {
        sock.recvFrom( (char *)message, MSGSIZE );
        //If ack and ACKseq equal, update cumulative ACK
        if(message[0] == i)
        {
          array[i] = true;
          //Find last true value in array[i]
          for(int index = max - 1;  index >=  0; index--)
          {
            //If the last true value is found
            if(array[index] == true)
            {
              //Set the ACK to current index, i to next index
                ack = index;
                i = index + 1;
                break;
            }
          }
        }
        //Mark the message as received
        else
        {
            array[message[0]] = true;
        }
        //Send back cumulative ACK
        sock.ackTo((char *)&ack, sizeof(ack));
    }
}

/* 
*server side window sliding with drop back case
*/
void serverEarlyRetransCase4(UdpSocket &sock, const int max, int *message, int windowSize, int dropRate) {

    // Will use the index i as the expected number within the message
    // Can only advance the loop if the data is received in correct order

    for (int i = 0; i < max; i++) {
        while (true) {
            if (sock.pollRecvFrom() > 0) {
                int percentage = rand() % 101; // Generate a random number from 0 -100;
                if (percentage < dropRate) {
                    continue;
                }
                sock.recvFrom((char *) message, MSGSIZE);
                sock.ackTo((char *) &i, sizeof(i));
                if (message[0] == i) {
                    break; // Can advance and receive the next data
                }
            }
        }

    }
}