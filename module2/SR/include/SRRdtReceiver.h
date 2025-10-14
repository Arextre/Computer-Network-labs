#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H

#include "Global.h"
#include "RdtReceiver.h"

#include <deque>
#include <cstdio>

using std::deque;

struct rcvPkt {
    bool rcvd;      // received flag
    Packet rcvPkt;
};

class SRRdtReceiver: public RdtReceiver {
private:
    int expectSequenceNumberRcvd;
    int base;
    int winlen;
    int seqlen;
    deque<rcvPkt> window;
    Packet lastAckPkt;

public:
    void receive(const Packet &packet) override;
    void printWindow(FILE *out, char split, char ends) const;

public:
    SRRdtReceiver();
    virtual ~SRRdtReceiver();

};

#endif