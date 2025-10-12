#ifndef GBN_RDT_SENDER_H
#define GBN_RDT_SENDER_H
#include "RdtSender.h"
class GBNRdtSender: public RdtSender {
private:
	int expectSequenceNumberSend;	// expected next sent seqnum 
	bool waitingState;				// whether at waiting ACL status
	Packet packetWaitingAck;		// packet sent and waited to be ACK

public:

	bool getWaitingState();
	bool send(const Message &message);						// 发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(const Packet &ackPkt);						// 接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					// Timeout handler，将被NetworkServiceSimulator调用

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif

