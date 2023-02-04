#include <iostream>
#include <iomanip>
#include <string>
const int length = 4;

using namespace std;

class Router {

public: 
    
    u_int32_t table[4][5] = {
        { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 8 },      
                { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 126 },    
                { 0x1, 0x1, 0x1, 0x1, 32 },                                 
                { 0x0, 0x0, 0x0, 0x0, 0 },
    };
    
    struct IPv6 {
        u_int version: 4;
        uint8_t trafficClass;
        u_int flowLabel: 20;
        uint16_t payloadLength;
        uint8_t nextHeader;
        uint8_t hopLimit;
        uint32_t src[4]; 
        uint32_t dest[4];
        void* payLoad;
        
        
        uint32_t* getSrcAdd() {
            return src;
        }
        void setSrcAdd(u_int32_t first, u_int32_t second, u_int32_t third, u_int32_t fourth) {
            src[0] = first;
            src[1] = second;
            src[2] = third;
            src[3] = fourth;
        }
        
        uint32_t* getDestAdd() {
            return dest;
        } 
        void setDestAdd(u_int32_t first, u_int32_t second, u_int32_t third, u_int32_t fourth) {
            dest[0] = first;
            dest[1] = second;
            dest[2] = third;
            dest[3] = fourth;
        }

        bool decreaseHopLimit() {
            u_int8_t curHop = hopLimit;
            hopLimit--;
            if (hopLimit < curHop) {
                return true;
            }
            return false;
            
        }

        void setHopLimit(u_int8_t hlimit) {
            hopLimit = hlimit;
        }
    };

    // determining where the coming packet goes
    string incomingIPv6(IPv6 p)
    {
        int numbits;
        // table match values
        int bestTM = 0;
        int bestTMBits = 0;
        u_int32_t shiftDest;
        u_int32_t shiftTable;

        if (!p.decreaseHopLimit()) {
            return "The packet from " + int32ArrToHexStr(p.src) + 
                   " was dropped due to hoplimit expiration ";
        }

        for (int i = 0; i < length; i++)
        {
            numbits = table[i][length];

            for (int j = 0; j < length; j++)
            {
                if (numbits > 32) {
                    if(p.dest[j] == table[i][j]) {
                        numbits -= 32;
                    }
                    else {
                        numbits = -1; 
                        break; // entry doesn't match so we break.
                    }
                }
                else {
                    if (numbits != 0) {
                        shiftTable = table[i][j];
                        shiftDest = p.dest[j];
                        shiftTable = shiftTable >> (32 - numbits);
                        shiftDest = shiftDest >> (32 - numbits);
                        numbits = 0;
                        if (shiftDest != shiftTable) {
                            numbits = -1;
                            break;
                        }
                    }
                }

                if(numbits == 0) { 
                    if((int)table[i][length] > bestTMBits) {
                        bestTMBits = table[i][length];
                        bestTM = i;
                    }
                    break;
                }
            }
        }

        if (bestTM == -1) {
            return "no outputs ";
        }
        else {
            return "The packet from " + int32ArrToHexStr(p.src) + " to "
                + int32ArrToHexStr(p.dest) +" was routed on interface " +
                to_string(bestTM) + '\n';
        }
    }

    string int32ArrToHexStr(u_int32_t address[length]) {
        stringstream stream;
        string out;
        string temp;
        for(int i = 0; i < length; i++) {
            stream.str(string());
            stream << hex << address[i];
            temp = stream.str();

            while(temp.length() < 8) {
                temp = "0" + temp;
            }

            //Formatting.
            temp.insert(4, ":");
            temp.append(":");
            out += temp;
        }

        out.erase(out.length()-1);
        return out;
    }
};
