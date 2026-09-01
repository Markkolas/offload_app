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

#ifndef SIMPLESERVER_H_
#define SIMPLESERVER_H_

#include <omnetpp.h>

#include "simple_m.h"
#include "L2multi_m.h"

#define MAX_CORES 128 //Dont want to use vectors

class SimpleServer : public omnetpp::cSimpleModule {
private:
    int CPU_CYCLES;
    int N_CORES;
    int QUEUE_SIZE;

    struct Core{
        omnetpp::cMessage *procEvent = nullptr; // May be a good idea to use smart pointers for this
        Simple_task *task = nullptr;
    }core[MAX_CORES];

    int fullq_events = 0;

    omnetpp::cGate *oGate;

    omnetpp::cQueue queue_buff{"server queue"};

    omnetpp::simsignal_t qLengthSig, qFullEvent;

protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    Simple_task * findAvailableTask(int core);
    omnetpp::simtime_t processTask(Simple_task *);
    bool saveTask(Simple_task *);
    void sendResult(Simple_task *);

public:
    ~SimpleServer();
};

#endif /* SIMPLESERVER_H_ */
