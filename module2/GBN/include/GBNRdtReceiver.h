#ifndef GBN_RDT_RECEIVER_H
#define GBN_RDT_RECEIVER_H
#include "RdtReceiver.h"
class GBNRdtReceiver: public RdtReceiver {
private:
	int expectSequenceNumberRcvd;	// expected sequence number of next in-order packet
	int seqlen;				   		// sequence number range
	Packet lastAckPkt;				// last sent acknowledgment packet

public:
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();

public:
	
	void receive(const Packet &packet);	// receive packet, invoked by NetworkService
};

#endif

