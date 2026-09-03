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
#include <memory>

/**
 * REGARDING SMART POINTERS IN OMNET++:
 *
 *  OMNET++ provides its own native memory management system. Part of this system is
 *  a hierarchical ownership mechanism that operates in the deletion of object. Each
 *  object that extends from cOwnedObject has a pointer which point to the object that
 *  owns it.
 *
 *  WELL BEHAVED OWNED OBJECTS ASKS THEIR OWNERS FOR PERMISION TO BE DELETED.
 *
 *  WELL BEHAVED OWNER OBJECTS DELETE ALL THEIR OWNED OBJECTS IN ITS DESTRUCTOR.
 *
 *  When a delete() is called upon an owned object, the object, inside its destructor,
 *  asks for permission to be destroyed to its owner. If the owner refuses, it typically
 *  ends with an exception throw.
 *
 *  This is mechanism is the main reason why smart pointers and standard C++ memory management
 *  are not typically used. Nevertheless smart pointers can be useful.
 *
 *  Care that when a raw pointer is obtained from the unique pointer and passed to a function,
 *  that function DOES not delete the pointer.
 */

#include "simple_m.h"
#include "L2multi_m.h"

#define MAX_CORES 128 //Dont want to use vectors

class SimpleServer : public omnetpp::cSimpleModule {
private:
    int CPU_CYCLES;
    int N_CORES;
    int QUEUE_SIZE;

    struct Core{
        std::shared_ptr<omnetpp::cMessage> procEvent = nullptr;
        std::shared_ptr<L2multi> packet = nullptr;
    }core[MAX_CORES];

    int fullq_events = 0;

    omnetpp::cQueue queue_buff{"server queue"};

    omnetpp::simsignal_t qLengthSig, qFullEvent;

protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    L2multi * findAvailablePacket(int core);
    omnetpp::simtime_t processTask(Simple_task *);
    bool savePacket(L2multi *);
    void sendResult(Simple_task *, int);

public:
    ~SimpleServer();
};

#endif /* SIMPLESERVER_H_ */
