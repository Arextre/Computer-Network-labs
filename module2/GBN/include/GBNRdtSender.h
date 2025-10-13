#ifndef GBN_RDT_SENDER_H
#define GBN_RDT_SENDER_H
#include "RdtSender.h"
#include <deque>
using std::deque;

class GBNRdtSender: public RdtSender {
private:
	int expectSequenceNumberSend;	// expected next sent seqnum 
	bool waitingState;				// whether at waiting ACK status
	int base;						// window base index
	int winlen;						// size of window
	int seqlen;						// seqnum range
	deque<Packet> window;			// queue of window, FIFO
	Packet packetWaitingAck;		// packet sent and waited to be ACK

public:

	bool getWaitingState();
	void printWindow(char split, char ends);										// Print the contents of current window
	bool send(const Message &msg);						// send packet from AppLayer, invoked by NetworkService Simulator. True if send sucessfully to NetworkLayer, else return false if at waiting-ACK status and refuse sending the packet.
	void receive(const Packet &ackPkt);						// receive ACK, invoked by NetworkServiceSimulator	
	void timeoutHandler(int seqnum);						// Timeout handler, invoked by NetworkServiceSimulator

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif

