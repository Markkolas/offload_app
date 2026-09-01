//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "SimpleServer.h"

using namespace omnetpp;

Define_Module(SimpleServer);

//Server implementation
void SimpleServer::initialize(){
    EV << "Server is alive!";
    qLengthSig = registerSignal("QueueLength");
    qFullEvent = registerSignal("QueueFull");
    oGate = gate("out");
}

void SimpleServer::handleMessage(cMessage *msg){
    if(strcmp(msg->getName(), "Test")==0){
        EV << "Message " << msg->getName() << " from client received! Deleting it \n";
    }
    else if(dynamic_cast<L2multi *>(msg) != nullptr){
        L2multi *L2packet = (L2multi *)msg; //TODO: Refractor this with smart pointers
        Simple_task *task = check_and_cast<Simple_task *>(L2packet->decapsulate());

        EV << "Task " << task->getName() << " received. Saving in task queue.\n";

        bool q_full = saveTask(task);

        if(q_full){
            EV << "Queue is full!" << endl;
            fullq_events++;
            emit(qFullEvent, fullq_events);
            delete(task);
        }
        else if (procEvent == nullptr){ // Queue was empty, start first q timer
            procEvent = new cMessage("ProcTimer");
            simtime_t pDelay = processTask(task);
            scheduleAfter(pDelay, procEvent);
        }

        delete(L2packet);
    }
    else if(msg == procEvent){
        // Check if is channel is available, else wait
        if(oGate->getChannel()->isBusy()){
            EV << "Channel is busy. Sender has to wait" << endl;
            scheduleAt(oGate->getChannel()->getTransmissionFinishTime(), procEvent);
        }
        else{
            EV << "Timer expired and channel empty, sending result\n";

            Simple_task *task = (Simple_task *)queue_buff.pop();

            sendResult(task);
            delete(task);

            if(queue_buff.getLength() == 0){ //Queue is empty, destroy timer
                delete(procEvent);
                procEvent = nullptr;
            }
            else{
                simtime_t pDelay = processTask((Simple_task *)queue_buff.front());
                scheduleAfter(pDelay, procEvent);
            }
        }
    }
    else{
        throw cRuntimeError("Ups, that should not happen. Message with name %s arrived", msg->getName());
    }
}

simtime_t SimpleServer::processTask(Simple_task *task){
    EV << "Number of bytes: " << task->getByteLength() << "\nComplexity factor: " << task->getComplexityFactor();
    EV << "\nStarting offload process\n";

    // I need to be carefull with data types in operations
    double ideal_delay = (double)(task->getByteLength())/(double)(CPU_CYCLES);
    simtime_t pDelay = simtime_t((ideal_delay*task->getComplexityFactor())/(1e3)); // milliseconds

    EV << "Processing will be completed after " << pDelay << " ms\n";
    return pDelay;
}

bool SimpleServer::saveTask(Simple_task *task){
    bool is_full = true;
    emit(qLengthSig, queue_buff.getLength());

    if(queue_buff.getLength() < QUEUE_SIZE){
        queue_buff.insert(task);
        is_full = false;
    }

    return is_full;
}

void SimpleServer::sendResult(Simple_task *task) {
    char resultName[20];
    snprintf(resultName, sizeof(resultName), "result-%d", task->getTaskId());

    Simple_result *result = new Simple_result(resultName);
    result->setResultId(task->getTaskId());
    result->setByteLength(uniform(1, 100));
    result->setTimestamp(getSimulation()->getSimTime());

    strcat(resultName, "-L2");
    L2multi *L2packet = new L2multi(resultName);
    L2packet->encapsulate(result);
    send(L2packet, "out");
}

SimpleServer::~SimpleServer(){
    cancelAndDelete(procEvent);
}
