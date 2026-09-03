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

    CPU_CYCLES = par("CPU_CYCLES").intValue();
    N_CORES = par("N_CORES").intValue();
    QUEUE_SIZE = par("QUEUE_SIZE").intValue();

    if(N_CORES > MAX_CORES)
        throw cRuntimeError("Configured cores exceeds maximum. Change maximum in sources.");

    // Create message timers
    for(int i = 0; i < N_CORES; i++){
        core[i].procEvent = std::make_shared<cMessage>("ProcTimer", i);
    }
}

void SimpleServer::handleMessage(cMessage *msg){
    if(strcmp(msg->getName(), "Test")==0){
        EV << "Message " << msg->getName() << " from client received! Deleting it \n";
    }
    else if(dynamic_cast<L2multi *>(msg) != nullptr){
        L2multi *L2packet = (L2multi *)msg;

        EV << "Packet " << L2packet->getName() << " received. Saving in task queue.\n";

        bool q_full = savePacket(L2packet);

        if(q_full){
            EV << "Queue is full!" << endl;
            fullq_events++;
            emit(qFullEvent, fullq_events);
            delete(L2packet);
        }
        else{ // Check if there are any idle cores. Note how cores always search for more tasks before idling
            for(int i = 0; i < N_CORES; i++){
                if(core[i].packet == nullptr){
                    core[i].packet.reset(L2packet); //Forced to do bad practice, OMNET++ does not combine well with shared pointers
                    simtime_t pDelay = processTask(check_and_cast<Simple_task *>(core[i].packet->getEncapsulatedPacket()));
                    scheduleAfter(pDelay, core[i].procEvent.get());
                    break; //Important
                }
            }
        }
    }
    else if(strcmp("ProcTimer", msg->getName()) == 0){
        int n_core = msg->getKind();
        int dest = core[n_core].packet->getSource();
        // Check if is channel is available, else wait
        if(gate("out", dest)->getChannel()->isBusy()){
            EV << "Channel is busy. Sender has to wait" << endl;
            scheduleAt(gate("out", dest)->getChannel()->getTransmissionFinishTime(), msg);
        }
        else{
            EV << "Timer expired and channel empty, CORE[" << n_core << "] sending a result" << endl;

            if(queue_buff.remove(core[n_core].packet.get()) == nullptr)
                throw cRuntimeError("Uh oh, a core tried to delete a nonexisting task");

            Simple_task *task = check_and_cast<Simple_task *>(core[n_core].packet.get()->decapsulate());

            sendResult(task, dest);
            delete(task);
            core[n_core].packet.reset(findAvailablePacket(n_core));

            if(core[n_core].packet != nullptr){
                simtime_t pDelay = processTask(check_and_cast<Simple_task *>(core[n_core].packet->getEncapsulatedPacket()));
                scheduleAfter(pDelay, msg);
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
    if(pDelay < 0){
        // Debug trap
        EV << "Debug trap" << endl;
    }
    return pDelay;
}

bool SimpleServer::savePacket(L2multi *packet){
    bool is_full = true;
    emit(qLengthSig, queue_buff.getLength());

    if(queue_buff.getLength() < QUEUE_SIZE){
        queue_buff.insert(packet);
        is_full = false;
    }

    return is_full;
}

void SimpleServer::sendResult(Simple_task *task, int dest) {
    char resultName[20];
    snprintf(resultName, sizeof(resultName), "result-%d", task->getTaskId());

    Simple_result *result = new Simple_result(resultName);
    result->setResultId(task->getTaskId());
    result->setByteLength(uniform(1, 100));
    result->setTimestamp(getSimulation()->getSimTime());

    strcat(resultName, "-L2");
    L2multi *L2packet = new L2multi(resultName);
    L2packet->encapsulate(result);
    send(L2packet, "out", dest);
}

L2multi * SimpleServer::findAvailablePacket(int n_core){
    for(cQueue::Iterator iter(queue_buff); !iter.end(); iter++){
        L2multi *packet = (L2multi*) *iter;
        bool is_executing = false;

        for(int i = 0; i<N_CORES; i++){
            if(i != n_core){
                if(packet == core[i].packet.get()){
                    is_executing = true;
                    break; // Pass to the next task
                }
            }
        }

        if(!is_executing){
            return packet;
        }
    }

    return nullptr;
}

SimpleServer::~SimpleServer(){
    for(int i=0; i<N_CORES; i++){
        cancelEvent(core[i].procEvent.get());
        core[i].procEvent = nullptr;

        queue_buff.remove(core[i].packet.get()); // Again, OMNET++ do not work well with smart pointers. Or its not trivial at least
        core[i].packet = nullptr;
    }
}
