/*
 * simple_app.cc
 *
 *  Created on: Aug 25, 2026
 *      Author: mava
 */

#include <omnetpp.h>

using namespace omnetpp;

class User : public cSimpleModule
{
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    int countMsg = 0;
    cMessage* timerEvent = nullptr;

public:
    ~User();
};

class Server : public cSimpleModule
{
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(User);
Define_Module(Server);

// User implementation
void User::initialize(){
    EV << "User is alive! Setting timer for 100ms\n";
    timerEvent = new cMessage("TimerEvent");
    scheduleAfter(0.1, timerEvent);
}

void User::handleMessage(cMessage *msg){
    if(msg == timerEvent){
        EV << "Timer expired, sending message\n";
        send(new cMessage("Test"), "out");
        scheduleAfter(exponential(0.1), timerEvent);
    }
    else{
        throw cRuntimeError("Ups, that should not happen. Message with name %s arrived", msg->getName());
    }
}

User::~User(){
    cancelAndDelete(timerEvent);
}

void Server::initialize(){
    EV << "Server is alive!";
}

void Server::handleMessage(cMessage *msg){
    if(strcmp(msg->getName(), "Test")==0){
        EV << "Message " << msg->getName() << " from client recieved! Deleting it\n";
        delete(msg);
    }
    else{
        throw cRuntimeError("Ups, that should not happen. Message with name %s arrived", msg->getName());
    }
}
