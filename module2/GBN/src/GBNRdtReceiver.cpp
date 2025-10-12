#include "Global.h"
#include "GBNRdtReceiver.h"

GBNRdtReceiver::GBNRdtReceiver(): expectSequenceNumberRcvd(0), seqlen(8) {
    lastAckPkt.acknum = -1;
    lastAckPkt.seqnum = -1; // ignored
    for (int i = 0; i < Configuration::PAYLOAD_SIZE; ++i) {
        lastAckPkt.payload[i] = '.';
    }
    lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

GBNRdtReceiver::~GBNRdtReceiver() {
}

void GBNRdtReceiver::receive(const Packet &packet) {
    int checksum = pUtils->calculateCheckSum(packet);
    if (checksum == packet.checksum && packet.seqnum == this->expectSequenceNumberRcvd) {
        // two conditions: 1. not corrupted; 2. seqnum is right;

        // extract and deliver
        Message msg;
        memcpy(msg.data, packet.payload, Configuration::PAYLOAD_SIZE);
        pns->delivertoAppLayer(RECEIVER, msg);
        
        // update lastAckPkt
        this->lastAckPkt.seqnum = packet.seqnum;
        lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
        pUtils->printPacket("GBNRdtReceiver acknowledge receive: ", lastAckPkt);

        // send ACK
        pns->sendToNetworkLayer(SENDER, lastAckPkt);

        // update expected seqnum
        this->expectSequenceNumberRcvd = (this->expectSequenceNumberRcvd + 1) % this->seqlen;
    } else {
        // refuse receiver
        if (checksum != packet.checksum) {
            // packet corrupt
            pUtils->printPacket("Packet refused (Packet Corrupt): ", packet);
        } else if (this->expectSequenceNumberRcvd != packet.seqnum) {
            pUtils->printPacket("Packet Refused (Packet Duplicated): ", packet);
        } else {
            pUtils->printPacket("Receiver Resend lastAckPkt: ", lastAckPkt);
            pns->sendToNetworkLayer(SENDER, lastAckPkt);
        }
    }
}