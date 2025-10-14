#ifndef TCP_GBN_RDT_RECEIVER_H
#define TCP_GBN_RDT_RECEIVER_H
#include "RdtReceiver.h"
class TCP_GBNRdtReceiver: public RdtReceiver {
private:
	int expectSequenceNumberRcvd;	// expected sequence number of next in-order packet
	int seqlen;				   		// sequence number range
	Packet lastAckPkt;				// last sent acknowledgment packet

public:
	TCP_GBNRdtReceiver();
	virtual ~TCP_GBNRdtReceiver();

public:
	
	void receive(const Packet &packet) override;	// receive packet, invoked by NetworkService
};

#endif

