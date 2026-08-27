/*
 * simple_app.cc
 *
 *  Created on: Aug 25, 2026
 *      Author: mava
 */

#include <omnetpp.h>

#include "simple_m.h"

using namespace omnetpp;

class User : public cSimpleModule
{
private:
    const int MIN_TASK_S = 100;
    const int MAX_TASK_S = 10000;
    cArray task_buffer{"tx_buff"};

    int countMsg = 0;
    int lastResult = 0;

    Simple_task * getTaskFromId(int Id);
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    cMessage *timerEvent = nullptr;

public:
    ~User();
};

class Server : public cSimpleModule
{
private:
    const int CPU_CYCLES = 3000; //MHz
    const int N_CORES = 8;
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    simtime_t processTask(Simple_task *);
};

Define_Module(User);
Define_Module(Server);

// User implementation
void User::initialize(){
    EV << "User is alive!";
    timerEvent = new cMessage("TimerEvent");
    scheduleAfter(0.1, timerEvent);
}

void User::handleMessage(cMessage *msg){
    if(msg == timerEvent){
        EV << "Timer expired, sending message\n";

        char taskName[20];
        snprintf(taskName, sizeof(taskName), "task-%d", countMsg);
        Simple_task *task = new Simple_task(taskName);
        task->setTaskId(countMsg++);
        task->setNumBytes(uniform(MIN_TASK_S, MAX_TASK_S));
        task->setComplexityFactor(normal(1, 0.2));
        task->setTimestamp();

        task_buffer.add(new Simple_task(*task)); // Store a copy. Beware of OMNET++ ownership
        send(task, "out");
        scheduleAfter(exponential(0.1), timerEvent);
    }
    else if(strstr(msg->getName(), "result") != nullptr){
        char bubbleMessage[100];
        snprintf(bubbleMessage, sizeof(bubbleMessage), "%s arrived!", msg->getName());
        bubble(bubbleMessage);

        EV << msg->getName() << " received. Finding offloading delay\n";

        Simple_result *result = check_and_cast<Simple_result *>(msg);
        Simple_task *ret_task = getTaskFromId(result->getResultId());

        simtime_t off_delay = result->getTimestamp() - ret_task->getTimestamp();

        EV << "--- Offloading delay: " << off_delay << " ---\n";

        delete(msg);
    }
    else{
        throw cRuntimeError("Ups, that should not happen. Message with name %s arrived", msg->getName());
    }
}

Simple_task * User::getTaskFromId(int Id){
    Simple_task *task = nullptr;

    for(int i = 0; i < task_buffer.size(); i++){
        if(task_buffer[i]){ //position used?
            task = check_and_cast<Simple_task *>(task_buffer[i]);
            if(task->getTaskId() == Id){
                task_buffer.remove(i); // Be careful with mod ops while iterating...
                break;
            }
        }
    }

    if(task){
        return task;
    }
    else{
        throw cRuntimeError("Something went wrong when searching task with task ID: %d", Id);
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
    }
    else if(strstr(msg->getName(), "task") != nullptr){
        EV << "Task " << msg->getName() << " received. Preparing to process.\n";

        Simple_task *task = check_and_cast<Simple_task *>(msg);
        EV << "Number of bytes: " << task->getNumBytes() << "\nComplexity factor: " << task->getComplexityFactor();
        EV << "\nStarting offload process\n";

        simtime_t pDelay = processTask(task);
        EV << "Processing will be completed after " << pDelay << " ms\n";

        char resultName[20];
        snprintf(resultName, sizeof(resultName), "result-%d", task->getTaskId());

        Simple_result *result = new Simple_result(resultName);
        result->setResultId(task->getTaskId());
        result->setNumBytes(uniform(1, 100));
        result->setTimestamp(getSimulation()->getSimTime() + pDelay);

        sendDelayed(result, pDelay, "out");
    }
    else{
        throw cRuntimeError("Ups, that should not happen. Message with name %s arrived", msg->getName());
    }
    delete(msg);
}

simtime_t Server::processTask(Simple_task *task){
    // I need to be carefull with data types in operations
    double ideal_delay = (double)(task->getNumBytes())/(double)(CPU_CYCLES);
    return simtime_t((ideal_delay*task->getComplexityFactor())/(1e3)); // milliseconds
}
