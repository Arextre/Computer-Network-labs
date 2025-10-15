#include "Global.h"
#include "TCP_GBNRdtSender.h"

#include <queue>
#include <cstdio>
#include <cassert>
using std::queue;

TCP_GBNRdtSender::TCP_GBNRdtSender():
        expectSequenceNumberSend(0), waitingState(false),
        base(0), winlen(4), seqlen(8) {
    while (!window.empty()) window.pop_back();
}

TCP_GBNRdtSender::~TCP_GBNRdtSender() {
}

bool TCP_GBNRdtSender::getWaitingState() {
    return this->waitingState = (this->window.size() == this->winlen);
}

void TCP_GBNRdtSender::printWindow(FILE *out = stdout,
                                   char split = ',',
                                   char ends = '\n') const {
    int len = static_cast<int>(this->window.size());
    for (int i = 0; i < len; ++i) {
        fprintf(out, "%d", (this->base + i) % this->seqlen);
        if (i != len - 1) fprintf(out, "%c", split);
    }
    fprintf(out, "%c", ends);
}

bool TCP_GBNRdtSender::send(const Message &msg) {
    if (getWaitingState())
        // sending window is full, refuse new msg
        return false;
    // make packet
    this->packetWaitingAck.acknum = -1;
    this->packetWaitingAck.seqnum = this->expectSequenceNumberSend;
    memcpy(this->packetWaitingAck.payload,
           msg.data,
           Configuration::PAYLOAD_SIZE);
    this->packetWaitingAck.checksum =
        pUtils->calculateCheckSum(this->packetWaitingAck);

    // send packet
    pUtils->printPacket("[Sender] Send packet: ", this->packetWaitingAck);
    if (this->base == this->expectSequenceNumberSend)
        // timer record the time cost of the first packet of window
        pns->startTimer(SENDER, Configuration::TIME_OUT, base);
    pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);

    // add packet to the window
    this->window.push_back(this->packetWaitingAck);

    // update expectSequenceNumberSend
    this->expectSequenceNumberSend =
        (this->expectSequenceNumberSend + 1) % this->seqlen;

    return true;
}

void TCP_GBNRdtSender::receive(const Packet &ackPkt) {
    int checksum = pUtils->calculateCheckSum(ackPkt);
    int offbase = (ackPkt.acknum - this->base + this->seqlen) % this->seqlen; // avoid negative
    int window_size = static_cast<int>(window.size());
    if (checksum == ackPkt.checksum && offbase < window_size) {
        // two conditions: 1. packet incorrupted; 2. acknum in window;
        
        pUtils->printPacket("[Sender] Receive ackPkt successfully: ", ackPkt);

        // stop timer
        pns->stopTimer(SENDER, base);

        // print the contents of window
        int acknum = ackPkt.acknum;
        FILE *windowlog = fopen("./logs/window_log_sender.txt", "a");
        fprintf(windowlog, "[Sender] base = %d, target = %d, window_size = %d\n", this->base, (acknum + 1) % this->seqlen, window_size);
        fprintf(windowlog, "[Sender] Contents of window before update: ");
        this->printWindow(windowlog);
        
        // update window
        while (this->base != (acknum + 1) % this->seqlen) {
            assert(!this->window.empty()); // window should not be empty
            this->window.pop_front();
            (this->base += 1) %= this->seqlen;
        }

        // print the contents of window after updating
        fprintf(windowlog, "[Sender] Contents of window after update: ");
        this->printWindow(windowlog);
        fprintf(windowlog, "[Sender] base = %d, window_size = %d\n", this->base, static_cast<int>(window.size()));

        // clear Redundant ACK counter when successfully receive ACK
        this->RdAck = 0;

        // restart timer if window not empty
        if (!window.empty())
            pns->startTimer(SENDER, Configuration::TIME_OUT, this->base);
    
    } else if (checksum != ackPkt.checksum) {
        pUtils->printPacket("[Sender] ackPkt refused (packet corrupt): ",
                            ackPkt);
    } else if (ackPkt.acknum == (this->base - 1 + this->seqlen) % this->seqlen) {
        // ACK number is the last ACKed packet
        pUtils->printPacket("[Sender] Redundant ACK received", ackPkt);
        this->RdAck++;
        if (this->RdAck == 3 && !window.empty()) {
            pUtils->printPacket("[Sender] Fast Recovery Mechanism Activate",
                                window.front());
            pns->sendToNetworkLayer(RECEIVER, window.front());
            this->RdAck = 0;
        }
    }
}

void TCP_GBNRdtSender::timeoutHandler(int seqnum) {
    pns->stopTimer(SENDER, seqnum);
    pns->startTimer(SENDER, Configuration::TIME_OUT, seqnum);
    int window_size = static_cast<int>(window.size());
    for (int i = 0; i < window_size; ++i) {
        pUtils->printPacket("[Sender] Time out, resend packet: ", window.at(i));
        pns->sendToNetworkLayer(RECEIVER, window.at(i));
    }
}