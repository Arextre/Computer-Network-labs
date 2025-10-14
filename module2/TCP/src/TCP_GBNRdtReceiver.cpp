#include "Global.h"
#include "TCP_GBNRdtReceiver.h"

TCP_GBNRdtReceiver::TCP_GBNRdtReceiver():
        expectSequenceNumberRcvd(0), seqlen(8) {
    lastAckPkt.acknum = -1;
    lastAckPkt.seqnum = -1; // ignored
    for (int i = 0; i < Configuration::PAYLOAD_SIZE; ++i) {
        lastAckPkt.payload[i] = '.';
    }
    lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

TCP_GBNRdtReceiver::~TCP_GBNRdtReceiver() {
}

void TCP_GBNRdtReceiver::receive(const Packet &packet) {
    int checksum = pUtils->calculateCheckSum(packet);
    if (
        checksum == packet.checksum
        && packet.seqnum == this->expectSequenceNumberRcvd
    ) {
        // two conditions: 1. packet incorrupted; 2. seqnum is right;

        // extract and deliver
        Message msg;
        memcpy(msg.data, packet.payload, Configuration::PAYLOAD_SIZE);
        pns->delivertoAppLayer(RECEIVER, msg);
        
        // update lastAckPkt
        this->lastAckPkt.acknum = packet.seqnum;
        lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
        pUtils->printPacket("[Receiver] Acknowledge receive: ", lastAckPkt);

        // send ACK and received seqnum
        pns->sendToNetworkLayer(SENDER, lastAckPkt);

        // update expected seqnum
        this->expectSequenceNumberRcvd =
            (this->expectSequenceNumberRcvd + 1) % this->seqlen;
    } else {
        // refuse receive
        if (checksum != packet.checksum) {
            // packet corrupt
            pUtils->printPacket("[Receiver] Packet refused " \
                                "(Packet Corrupt): ", packet);
        } else if (this->expectSequenceNumberRcvd != packet.seqnum) {
            // unknown or duplicate packet
            pUtils->printPacket("[Receiver] Packet Refused " \
                                "(Packet Duplicated): ", packet);
        }
        // default behavior, udt_sent
        pUtils->printPacket("[Receiver] Resend lastAckPkt " \
                            "(Default Behavior): ", lastAckPkt);
        pns->sendToNetworkLayer(SENDER, lastAckPkt);
    }
}