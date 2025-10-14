#include "Global.h"
#include "SRRdtSender.h"
#include "SRRdtReceiver.h"

#include <cstdio>
#include <deque>
using std::deque;

int main(int args, char* argv[]) {
    RdtSender *sender = new SRRdtSender();
    RdtReceiver *receiver = new SRRdtReceiver();
    pns->setRunMode(0);
    pns->init();
    pns->setRtdSender(sender);
    pns->setRtdReceiver(receiver);
    pns->setInputFile("./input.txt");
    pns->setOutputFile("./result.txt");

    pns->start();

    delete sender;
    delete receiver;
    delete pUtils;
    delete pns;
    
    return 0;
}