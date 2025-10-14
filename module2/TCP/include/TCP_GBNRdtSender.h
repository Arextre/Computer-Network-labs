#ifndef TCP_GBN_RDT_SENDER_H
#define TCP_GBN_RDT_SENDER_H

#include "RdtSender.h"

#include <deque>
#include <cstdio>
using std::deque;

class TCP_GBNRdtSender: public RdtSender {
private:
	int expectSequenceNumberSend;	// expected next sent seqnum 
	bool waitingState;				// whether at waiting ACK status
	int base;						// window base index
	int winlen;						// size of window
	int seqlen;						// seqnum range
	deque<Packet> window;			// queue of window, FIFO
	Packet packetWaitingAck;		// packet sent and waited to be ACK

public:

	bool getWaitingState() override;
	bool send(const Message &msg) override;						        // send packet from AppLayer, invoked by NetworkService Simulator. True if send sucessfully to NetworkLayer, else return false if at waiting-ACK status and refuse sending the packet.
	void receive(const Packet &ackPkt) override;						// receive ACK, invoked by NetworkServiceSimulator	
	void timeoutHandler(int seqnum) override;						    // Timeout handler, invoked by NetworkServiceSimulator

public:
	void printWindow(FILE *out, char split, char ends) const;						// Print the contents of current window

public:
	TCP_GBNRdtSender();
	virtual ~TCP_GBNRdtSender();
};

#endif

