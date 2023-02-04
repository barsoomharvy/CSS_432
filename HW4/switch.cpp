#include <string>
#include <iomanip>
#include <iostream>
#include <vector>
using namespace std;

class Switch {

public:

    struct EthernetFrame {
        u_int64_t preamble: (8*7);  
        u_int SFD: (8*1); 

        u_int16_t dest[3];
        u_int16_t src[3];
        u_int16_t length; 

        char payload[375]; 
        u_int32_t CRC;

        EthernetFrame() {
            preamble = 0;
            SFD = 171; 
            length = 1500;
            for (int i = 0; i < 375; i++) { payload[i] = 'L'; }
            CRC = 2147483647;
        }

        u_int16_t* getSrcAdd() {
            return src;
        }

        void setSrcAdd(u_int16_t first, u_int16_t second, u_int16_t third) {
            src[0] = first;
            src[1] = second;
            src[2] = third;
        }
        
        u_int16_t* getDestAdd() {
            return dest;
        }

        void setDestAdd(u_int16_t first, u_int16_t second, u_int16_t third) {
            dest[0] = first;
            dest[1] = second;
            dest[2] = third;
        }
    };

    
    string setIncomingFrame(EthernetFrame frame, int interface) {
        // Binding in the routing table the interface number to the source addr.
        if (!checkEntry(frame.src, interface)) 
            addToTable(frame.src, interface);
 

        int outInterface = checkTable(frame.dest);
        string ret = "The frame from " + int16ArrToHexStr(frame.src) + " to " +
        int16ArrToHexStr(frame.dest) + " was switched onto ";

        if (outInterface == -1) {
            ret += "all interfaces except " + to_string(interface) + '.';
        }
        else {
            ret += "the interface " + to_string(outInterface) + '.';
        }
        cout << endl;
        return ret;
    }


private:
    struct addr {
        u_int16_t addr[3];
    };

    vector<addr> routingTable[4];

    int checkTable(u_int16_t addr[3]) {
        for(int i = 0; i < 4; i++) {
            if(checkEntry(addr, i)) {
                return i;
            }
        }
        return -1;
    }

    //checking if addr bound to interface.
    bool checkEntry(u_int16_t addr[3], int interface) {

        for(int i = 0; i < routingTable[interface].size(); i++) {

            if( routingTable[interface].at(i).addr[0] == addr[0] &&
                routingTable[interface].at(i).addr[1] == addr[1] &&
                routingTable[interface].at(i).addr[2] == addr[2]) {

                return true;
            }
        }
        return false;
    }

    //Adding the addr and interface to table.
    void addToTable(u_int16_t addr[3], int interface) {
        Switch::addr temp;
        temp.addr[0] = addr[0];
        temp.addr[1] = addr[1];
        temp.addr[2] = addr[2];

        routingTable[interface].push_back(temp);
    }

    //Converting u_int16_t array to hex string.
    string int16ArrToHexStr(u_int16_t addr[3]) {
        stringstream stream;
        string output;
        string temp;
        for(int i = 0; i < 3; i++) {
            stream.str(string());
            stream << hex << addr[i];
            temp = stream.str();
            while(temp.length() < 4) {
                temp = "0" + temp;
            }
            //Formatting.
            temp.insert(2, ":");
            temp.append(":");
            output += temp;
        }
        output.erase(output.length()-1);
        return output;
    }

};