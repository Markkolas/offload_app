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
}

void SimpleServer::handleMessage(cMessage *msg){
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

simtime_t SimpleServer::processTask(Simple_task *task){
    // I need to be carefull with data types in operations
    double ideal_delay = (double)(task->getNumBytes())/(double)(CPU_CYCLES);
    return simtime_t((ideal_delay*task->getComplexityFactor())/(1e3)); // milliseconds
}
