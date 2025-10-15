#include "Global.h"
#include "TCP_GBNRdtSender.h"
#include "TCP_GBNRdtReceiver.h"

int main(int argc, char* argv[]) {
    // create environment
    RdtSender *sender = new TCP_GBNRdtSender();
    RdtReceiver *receiver = new TCP_GBNRdtReceiver();
    pns->setRunMode(0); // verbose mode
    pns->init();
    pns->setRtdSender(sender);
    pns->setRtdReceiver(receiver);
    pns->setInputFile("./input.txt");
    pns->setOutputFile("./result.txt");
    
    // start
    pns->start();

    delete sender;
    delete receiver;
    delete pUtils;
    delete pns;

    return 0;
}