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

#include "SimpleUser.h"

using namespace omnetpp;

Define_Module(SimpleUser);

// User implementation
void SimpleUser::initialize(){
    EV << "User is alive!";
    offDelaySig = registerSignal("OffDelay");
    timerEvent = new cMessage("TimerEvent");
    oGate = gate("out");
    scheduleAfter(1/par("tasksPerSecond").doubleValue(), timerEvent);
}

void SimpleUser::handleMessage(cMessage *msg){
    if(msg == timerEvent){
        // Check if is channel is available, else wait
        if(oGate->getChannel()->isBusy()){
            EV << "Channel is busy. Sender has to wait" << endl;
            scheduleAt(oGate->getChannel()->getTransmissionFinishTime(), timerEvent);
        }
        else{
            EV << "Timer expired and channel empty, sending task\n";

            char taskName[20];
            snprintf(taskName, sizeof(taskName), "task-%d", countMsg);
            Simple_task *task = new Simple_task(taskName);
            task->setTaskId(countMsg++);
            task->setByteLength(uniform(MIN_TASK_S, MAX_TASK_S));
            task->setComplexityFactor(truncnormal(1, 0.2));
            task->setTimestamp();

            task_buffer.add(new Simple_task(*task)); // Store a copy. Beware of OMNET++ ownership

            strcat(taskName, "-L2");
            L2multi *L2packet = new L2multi(taskName);
            L2packet->setSource(getIndex());
            L2packet->encapsulate(task);
            send(L2packet, "out");
            scheduleAfter(1/par("tasksPerSecond").doubleValue(), timerEvent);
        }
    }
    else if(dynamic_cast<L2multi *>(msg) != nullptr){
        char bubbleMessage[100];
        L2multi *L2packet = (L2multi *)msg; //TODO: Refractor this with smart pointers
        Simple_result *result = check_and_cast<Simple_result *>(L2packet->decapsulate());

        snprintf(bubbleMessage, sizeof(bubbleMessage), "%s arrived!", result->getName());
        bubble(bubbleMessage);

        EV << result->getName() << " received. Finding offloading delay\n";

        Simple_task *ret_task = getTaskFromId(result->getResultId());

        simtime_t off_delay = result->getTimestamp() - ret_task->getTimestamp();

        EV << "--- Offloading delay: " << off_delay << " ms ---\n";

        emit(offDelaySig, off_delay.dbl());

        delete L2packet;
        delete result;
        delete ret_task;
    }
    else{
        throw cRuntimeError("Ups, that should not happen. Message with name %s arrived", msg->getName());
    }
}

Simple_task * SimpleUser::getTaskFromId(int Id){
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

SimpleUser::~SimpleUser(){
    cancelAndDelete(timerEvent);
}
