/*
 * simple_app.cc
 *
 *  Created on: Aug 25, 2026
 *      Author: mava
 */

#include <omnetpp.h>

#include "simple_task_m.h"

using namespace omnetpp;

class User : public cSimpleModule
{
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    int countMsg = 0;
    cMessage *timerEvent = nullptr;

public:
    ~User();
};

class Server : public cSimpleModule
{
private:
    const long cpuCycles = 3*(1e9);
    const int numCores = 8;
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    simtime_t processTask(Simple_task *);
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

        char taskName[20];
        snprintf(taskName, sizeof(taskName), "task-%d", countMsg);
        Simple_task *task = new Simple_task(taskName);
        task->setTaskCounter(countMsg++);

        send(task, "out");
        scheduleAfter(exponential(0.1), timerEvent);
    }
    else if(strstr(msg->getName(), "result") != nullptr){
        char bubbleMessage[100];
        snprintf(bubbleMessage, sizeof(bubbleMessage), "%s arrived!", msg->getName());
        bubble(bubbleMessage);
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
        EV << "Message " << msg->getName() << " from client received! Deleting it \n";
        delete(msg);
    }
    else if(strstr(msg->getName(), "task") != nullptr){
        EV << "Task " << msg->getName() << " received. Preparing to process.\n";

        Simple_task *task = check_and_cast<Simple_task *>(msg);
        EV << "Number of bytes: " << task->getNumBytes() << "\nComplexity factor: " << task->getComplexityFactor();
        EV << "Starting offload process\n";

        simtime_t pDelay = processTask(task);
        EV << "Processing will be completed after " << pDelay << "seconds\n";
        char resultName[20];
        snprintf(resultName, sizeof(resultName), "result-%d", task->getTaskCounter());
        sendDelayed(new cMessage(resultName), pDelay, "out");
    }
    else{
        throw cRuntimeError("Ups, that should not happen. Message with name %s arrived", msg->getName());
    }
}

simtime_t Server::processTask(Simple_task *task){
    return simtime_t((task->getNumBytes()/cpuCycles)*task->getComplexityFactor());
}
