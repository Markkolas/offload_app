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
    qLengthSig = registerSignal("QueueLength");
    qFullEvent = registerSignal("QueueFull");


    timerEvent = std::make_shared<cMessage>("TimerEvent");
    txCompleteEvent = new cMessage("TxCompleteEvent");

    oGate = gate("out");

    CPU_CYCLES = par("CPU_CYCLES").intValue();
    QUEUE_SIZE = par("QUEUE_SIZE").intValue();

    MIN_TASK_S = par("MIN_TASK_S").intValue();
    MAX_TASK_S = par("MAX_TASK_S").intValue();

    posx = par("posx").intValue();
    posy = par("posy").intValue();
    x = posx;
    y = posy;
    pivotRadius = par("pivotRadius").intValue();
    velocity = par("velocity").doubleValue();
    heading = par("heading").doubleValue();

    txPower = par("txPower").doubleValue();

    Loss_coeff = pow((3*1e8)/(4*M_PI*2.4*1e9), 2); // Hardcoded pathloss f=2.4GHz

    if(getParentModule()->isVector() == 0)
        NODE_ID = getIndex();
    else
        NODE_ID = 0;

    scheduleAt(getSimulation()->getSimTime(), timerEvent.get());
}

void SimpleUser::handleMessage(cMessage *msg){
    if(msg == timerEvent.get()){
        // How many tasks?
        for(int i = 0; i < (int)par("traffic_gen").doubleValue(); i++){
            Simple_task *task = genTask();

            bool q_full = saveTask(task);

            if(q_full){
                EV << "Task queue in user is full!" << endl;
                fullq_events++;
                emit(qFullEvent, fullq_events);

                delete task;
            }
        }

        scheduleAfter(par("tasksGenPeriod").doubleValue(), msg);

        // If queue has packets and no packet is being send, send first on queue
        if(task_buffer.getLength() > 0 && txCompleteEvent->isScheduled() == false)
            scheduleAt(getSimulation()->getSimTime(), txCompleteEvent);
    }
    else if(msg == txCompleteEvent){

        // Check if channel is available
        if(oGate->getChannel()->isBusy())
            throw cRuntimeError("Channel is busy for transmission. This should never happen.");

        if(task_buffer.getLength() > 0){
            sendTask(check_and_cast<Simple_task *>(task_buffer.pop())); //TODO: pop() or front()??
            scheduleAt(oGate->getChannel()->getTransmissionFinishTime(), msg);
        }
    }
    else if(dynamic_cast<L2multi *>(msg) != nullptr){
        char bubbleMessage[100];
        L2multi *L2packet = (L2multi *)msg;
        Simple_result *result = check_and_cast<Simple_result *>(L2packet->decapsulate());

        snprintf(bubbleMessage, sizeof(bubbleMessage), "%s arrived!", result->getName());
        bubble(bubbleMessage);

        processResult(result);

        delete L2packet;
        delete result;
    }
    else{
        throw cRuntimeError("Ups, that should not happen. Message with name %s arrived", msg->getName());
    }
}


Simple_task * SimpleUser::genTask(){
    char taskName[20];
    snprintf(taskName, sizeof(taskName), "%d-task-%d", NODE_ID, countMsg);
    Simple_task *task = new Simple_task(taskName);
    task->setTaskId(countMsg++);
    task->setByteLength(uniform(MIN_TASK_S, MAX_TASK_S));
    task->setComplexityFactor(truncnormal(1, 0.2));
    task->setTimestamp();

    return task;
}

void SimpleUser::sendTask(Simple_task *task){
    char packetName[20];
    strcpy(packetName, task->getName());
    strcat(packetName, "-L2");
    L2multi *L2packet = new L2multi(packetName);
    L2packet->setSource(NODE_ID);
    L2packet->encapsulate(task);

    pivotAround(); // Move before transmitting
    cDatarateChannel *ch = check_and_cast<cDatarateChannel *>(oGate->getChannel());
    ch->setDatarate(1e6*log2(1+(txPower/n0)*(Loss_coeff/(x*x+y*y)))); // Channel cap. assuming free space loss model
    send(L2packet, oGate);
}

bool SimpleUser::saveTask(Simple_task *task){
    bool is_full = true;
    emit(qLengthSig, task_buffer.getLength());

    if(task_buffer.getLength() < QUEUE_SIZE){
        task_buffer.insert(task);
        is_full = false;
    }

    return is_full;
}

void SimpleUser::processResult(Simple_result *result){
    EV << result->getName() << " received. Finding offloading delay\n";

    simtime_t off_delay = getSimulation()->getSimTime() - result->getTimestamp();

    EV << "--- Offloading delay: " << off_delay*1000 << " ms ---";

    emit(offDelaySig, off_delay.dbl());
}

void SimpleUser::pivotAround(){
    // Just move within a square
    x = posx + uniform(-pivotRadius, +pivotRadius);
    y = posy + uniform(-pivotRadius, +pivotRadius);

    while(x == 0 && y == 0){
        x = posx + uniform(-pivotRadius, +pivotRadius);
        y = posy + uniform(-pivotRadius, +pivotRadius);
    }
}

SimpleUser::~SimpleUser(){
    cancelEvent(timerEvent.get());
    cancelEvent(txCompleteEvent);
    timerEvent = nullptr; // Smart pointer working in a very unamazing way
    delete txCompleteEvent;
}

//Simple_task * SimpleUser::getTaskFromId(int Id){
//    Simple_task *task = nullptr;
//
//    for(int i = 0; i < task_buffer.size(); i++){
//        if(task_buffer[i]){ //position used?
//            task = check_and_cast<Simple_task *>(task_buffer[i]);
//            if(task->getTaskId() == Id){
//                task_buffer.remove(i); // Be careful with mod ops while iterating...
//                break;
//            }
//        }
//    }
//
//    if(task){
//        return task;
//    }
//    else{
//        throw cRuntimeError("Something went wrong when searching task with task ID: %d", Id);
//    }
//
//}
