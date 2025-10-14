#include "Global.h"
#include "GBNRdtSender.h"
#include "GBNRdtReceiver.h"

int main(int argc, char* argv[]) {
    // create environment
    RdtSender *sender = new GBNRdtSender();
    RdtReceiver *receiver = new GBNRdtReceiver();
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