#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H

#include "Global.h"
#include "RdtSender.h"

#include <cstdio>
#include <deque>

using std::deque;

struct sendPkt {
    bool isAcked;   // whether the packet is ACKed
    Packet ackPkt;
};

class SRRdtSender: public RdtSender {
private:
    int expectSequenceNumberSend;
    bool waitingState;
    int base;                       // send base
    int winlen;
    int seqlen;
    deque<sendPkt> window;
    Packet pktWaitingAck;

public:
    bool getWaitingState() override;
    bool send(const Message &msg) override;
    void receive(const Packet &ackPkt) override;
    void timeoutHandler(int seqnum) override;

public:
    void printWindow(FILE *out, char split, char ends) const;

public:
    SRRdtSender();
    virtual ~SRRdtSender();

};

#endif