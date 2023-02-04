#include <iostream>
#include "router.cpp"
#include "switch.cpp"
using namespace std;

int main() {
    Switch Switch;
    Router Router;

    //Switch TEST:

    Switch::EthernetFrame frame;

    cout << "SWITCH TEST:"  << endl << endl;

    //Show we can access the elements.
    u_int16_t* frameSrcPtr = frame.getSrcAdd();
    u_int16_t* frameDestPtr = frame.getDestAdd();

    //switch test.

    frame.setSrcAdd(0xffff, 0xffff, 0x1);
    frame.setDestAdd(0x80, 0xf02, 0x2692);
    cout << Switch.setIncomingFrame(frame, 0);
    cout << endl;

    // a frame IN TABLE sending to frame not in table
    frame.setSrcAdd(0xffff, 0xffff, 0x2);
    frame.setDestAdd(0x80, 0xf02, 0x2692);
    cout << Switch.setIncomingFrame(frame, 0);
    cout << endl;

    // a frame being sent to a place with 2 frames.
    frame.setSrcAdd(0xffff, 0xffff, 0x3);
    frame.setDestAdd(0xffff, 0xffff, 0x1);
    cout << Switch.setIncomingFrame(frame, 1);
    cout << endl;

    // a frame being sent to a place with 2 frames, choosing latter one.
    frame.setSrcAdd(0xffff, 0xffff, 0x4);
    frame.setDestAdd(0xffff, 0xffff, 0x2);
    cout << Switch.setIncomingFrame(frame, 2);
    cout << endl;

     // send something to interface 1.
    frame.setSrcAdd(0xffff, 0xffff, 0x5);
    frame.setDestAdd(0xffff, 0xffff, 0x3);
    cout << Switch.setIncomingFrame(frame, 3);
    cout << endl;

    // sending to a normal interface, no stacking
    frame.setSrcAdd(0xffff, 0xffff, 0x5);
    frame.setDestAdd(0xffff, 0xffff, 0x4);
    cout << Switch.setIncomingFrame(frame, 3);
    cout << endl;


    // send something to interface 3.
    frame.setSrcAdd(0xffff, 0xffff, 0x4);
    frame.setDestAdd(0xffff, 0xffff, 0x5);
    cout << Switch.setIncomingFrame(frame, 2);
    cout << endl;
    cout << endl;

    //ROUTER TEST:

    Router::IPv6 packet;

    cout << "ROUTER TEST: " << endl << endl;

    //Show we can access the elements.
    u_int32_t* packetSourcePointer = packet.getSrcAdd();
    u_int32_t* packetDestinationPointer = packet.getDestAdd();
    
    //For completeness, use interface 0.
    packet.setSrcAdd(0x210e, 0x1aba, 0x10ac, 0xa7c);
    packet.setDestAdd(0xFFAAAAAA, 0x0, 0x0, 0x0);
    packet.setHopLimit(8);
    cout << Router.incomingIPv6(packet);
    cout << endl;

    // choosing the interface with the better match.
    packet.setSrcAdd(0x2aeb, 0x28e, 0x141, 0x0);
    packet.setDestAdd(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
    packet.setHopLimit(8);
    cout << Router.incomingIPv6(packet);
    cout << endl;

    // the catch-all.
    packet.setSrcAdd(0x539, 0x539, 0x539, 0x539);
    packet.setDestAdd(0x80, 0x80, 0x80, 0x80);
    packet.setHopLimit(8);
    cout << Router.incomingIPv6(packet);
    cout << endl;

    // an address that is partly correct.
    packet.setSrcAdd(0x4d2, 0x162e, 0x8adf38, 0xc8860c);
    packet.setDestAdd(0x1, 0x2, 0x2, 0x2);
    packet.setHopLimit(8);
    cout << Router.incomingIPv6(packet);
    cout << endl;

    //Final test for normal functionality.
    packet.setSrcAdd(0x210e, 0x1aba, 0x10ac, 0xa7c);
    packet.setDestAdd(0x1, 0x1, 0x1, 0x1);
    packet.setHopLimit(8);
    cout << Router.incomingIPv6(packet);
    cout << endl;

     // hop limit.
    packet.setSrcAdd(0x2aeb, 0x28e, 0x141, 0x0);
    packet.setDestAdd(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
    packet.setHopLimit(0);
    cout << Router.incomingIPv6(packet);
    cout << endl;

}