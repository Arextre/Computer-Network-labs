#include "Global.h"
#include "SRRdtSender.h"

#include <deque>
#include <cstdio>
#include <cassert>
using std::deque;

SRRdtSender::SRRdtSender(): expectSequenceNumberSend(0),
                            waitingState(false), base(0),
                            winlen(4), seqlen(8) {
    window.clear();
}

SRRdtSender::~SRRdtSender() {
}

bool SRRdtSender::getWaitingState() {
    return waitingState = (static_cast<int>(window.size()) == winlen);
}

void SRRdtSender::printWindow(FILE *out = stdout,
                              char split = ',',
                              char ends = '\n') const {
    fprintf(out, "[Sender] Window size = %d, contents: ", winlen);
    int win_size = static_cast<int>(window.size());
    for (int i = 0; i < winlen; ++i) {
        if (i < win_size) {
            fprintf(out, "%2d(%c)",
                    (this->base + i) % this->seqlen,
                    "01"[window[i].isAcked]);
        } else {
            fprintf(out, "--(-)");
        }
        if (i != winlen - 1)
            fprintf(out, "%c", split);
    }
    fprintf(out, "%c", ends);
}

bool SRRdtSender::send(const Message &msg) {
    if (getWaitingState()) {
        // log info print
        printf("[Sender] Message refused (window full)\n");
        return false;
    }
    // make packet
    this->pktWaitingAck.acknum = -1; // ignored;
    this->pktWaitingAck.seqnum = this->expectSequenceNumberSend;
    memcpy(this->pktWaitingAck.payload, msg.data, Configuration::PAYLOAD_SIZE);
    this->pktWaitingAck.checksum = pUtils->calculateCheckSum(this->pktWaitingAck);

    // send packet
    window.push_back(sendPkt{false, this->pktWaitingAck});
    pUtils->printPacket("[Sender] Packet send", this->pktWaitingAck);
    pns->startTimer(SENDER, Configuration::TIME_OUT, this->pktWaitingAck.seqnum);
    pns->sendToNetworkLayer(RECEIVER, this->pktWaitingAck);
    this->expectSequenceNumberSend = 
        (this->expectSequenceNumberSend + 1) % this->seqlen;
    return true;
}

void SRRdtSender::receive(const Packet &ackPkt) {
    int checksum = pUtils->calculateCheckSum(ackPkt);
    int offset = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen;
    if (checksum == ackPkt.checksum
        && offset < winlen
        && window[offset].isAcked == false) {
        // log info print
        pUtils->printPacket("[Sender] ACK received successfully", ackPkt);
        FILE *windowlog = fopen("./logs/window_log_sender.txt", "a");
        fprintf(windowlog, "[Sender] offset = %d, base = %d, "\
                           "window before update:\n", offset, this->base);
        this->printWindow(windowlog);

        // update window and stop timer
        window[offset].isAcked = true;
        pns->stopTimer(SENDER, ackPkt.acknum);
        // slide window
        while (!window.empty() && window.front().isAcked) {
            window.pop_front();
            (this->base += 1) %= this->seqlen;
        }

        fprintf(windowlog, "[Sender] Window after update:\n");
        this->printWindow(windowlog);
        fclose(windowlog);

    } else if (checksum != ackPkt.checksum) {
        pUtils->printPacket("[Sender] ACK refused (corrupt)", ackPkt);
    } else {
        pUtils->printPacket("[Sender] ACK refused (duplicate)", ackPkt);
    }
}

void SRRdtSender::timeoutHandler(int seqnum) {
    // resend seqnum packet
    printf("[Sender] Timeout, seqnum = %d\n", seqnum);
    pns->stopTimer(SENDER, seqnum);
    int offset = (seqnum - this->base + this->seqlen) % this->seqlen;
    assert(offset < static_cast<int>(window.size()));
    pUtils->printPacket("[Sender] Resend Packet", window[offset].ackPkt);
    pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
    pns->sendToNetworkLayer(RECEIVER, window[offset].ackPkt);
}
