#include "Global.h"
#include "SRRdtReceiver.h"

SRRdtReceiver::SRRdtReceiver(): expectSequenceNumberRcvd(0), base(0),
                                     winlen(4), seqlen(8) {
    lastAckPkt.seqnum = -1; // ignored
    lastAckPkt.acknum = -1; // for the first packet error
    for (int i = 0; i < Configuration::PAYLOAD_SIZE; ++i)
        lastAckPkt.payload[i] = '.';
    lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
    window.resize(winlen, rcvPkt{false, Packet()});
}

SRRdtReceiver::~SRRdtReceiver() {
}

void SRRdtReceiver::printWindow(FILE *out = stdout,
                                char split = ',',
                                char ends = '\n') {
    fprintf(out, "[Sender] Window size = %d, contents: ", winlen);
    for (int i = 0; i < winlen; ++i) {
        printf("%d(%c)", this->base + i, "01"[window[i].rcvd]);
        if (i != winlen - 1)
            putchar(split);
    }
    putchar(ends);
}

void SRRdtReceiver::receive(const Packet &packet) {
    int checksum = pUtils->calculateCheckSum(packet);
    int offset = (packet.seqnum - this->base + this->seqlen) % this->seqlen;

    if (checksum == packet.checksum
        && offset < winlen
        && window[offset].rcvd == false) {
        
        // log info print
        pUtils->printPacket("[Receiver] Packet Received Successfully", packet);
        fprintf(stdout, "[Receiver]: offset = %d, " \
                        "window before update: \n", offset);
        this->printWindow();

        // store the packet in the buffer window
        window[offset].rcvd = true;
        window[offset].rcvPkt = packet;

        while (window.front().rcvd == true) {
            // fetch packet and send to AppLayer
            Message msg;
            memcpy(msg.data,
                   window.front().rcvPkt.payload,
                   Configuration::PAYLOAD_SIZE);
            pns->delivertoAppLayer(RECEIVER, msg);
            

            // move window
            (this->base += 1) %= this->seqlen;
            window.pop_front();
            window.push_back(rcvPkt{false, Packet()});
        }

        fprintf(stdout, "[Receiver] Window after update: ");
        this->printWindow();

        // send ACK to sender
        lastAckPkt.acknum = packet.seqnum;
        lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
        pUtils->printPacket("[Receiver] Send ACK to sender", lastAckPkt);
        pns->sendToNetworkLayer(SENDER, lastAckPkt);

    } else {
        if (checksum != packet.checksum) {
            pUtils->printPacket("[Receiver] Packet Refused (corrupt)", packet);
        } else {
            pUtils->printPacket("[Receiver] Packet Refused (duplicate)", packet);
            lastAckPkt.acknum = packet.seqnum;
            lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
            pUtils->printPacket("[Receiver] Resent ACK to sender", lastAckPkt);
            pns->sendToNetworkLayer(SENDER, lastAckPkt);
        }
    }
}

