#ifndef GBN_RDT_SENDER_H
#define GBN_RDT_SENDER_H
#include "RdtSender.h"
class GBNRdtSender: public RdtSender {
private:
	int expectSequenceNumberSend;	// expected next sent seqnum 
	bool waitingState;				// whether at waiting ACK status
	Packet packetWaitingAck;		// packet sent and waited to be ACK

public:

	bool getWaitingState();
	bool send(const Message &message);						// send packet from AppLayer, invoked by NetworkService Simulator. True if send sucessfully to NetworkLayer, else return false if at waiting-ACK status and refuse sending the packet.
	void receive(const Packet &ackPkt);						// receive ACK, invoked by NetworkServiceSimulator	
	void timeoutHandler(int seqNum);						// Timeout handler, invoked by NetworkServiceSimulator

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif

