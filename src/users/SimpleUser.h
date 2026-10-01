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

#ifndef SIMPLEUSER_H_
#define SIMPLEUSER_H_

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
 *  WELL BEHAVED OWNER OBJECTS DELETE ALL THEIR OWNED OBJECTS IN ITS DESTRUCTOR (sometimes they release
 *  them all before destroying themselves).
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

#include "messages/simple_m.h"
#include "messages/L2multi_m.h"

class SimpleUser : public omnetpp::cSimpleModule  {
private:
    int CPU_CYCLES;
    int QUEUE_SIZE;

    int NODE_ID;

    int MIN_TASK_S, MAX_TASK_S;

    int posx, posy, x, y;
    int pivotRadius;
    double velocity, heading;

    double txPower;
    const double n0 = 1.38*1e-23*294*1e6; //Hardcoded T=21ºC and B=1MHz noise
    double Loss_coeff;

    omnetpp::cQueue task_buffer{"task_buff"};
    omnetpp::cGate *oGate;

    int countMsg = 0, fullq_events = 0;

    std::shared_ptr<omnetpp::cMessage> timerEvent = nullptr;
    omnetpp::cMessage *txCompleteEvent = nullptr;

    //Simple_task * getTaskFromId(int Id);

    omnetpp::simsignal_t offDelaySig, qLengthSig, qFullEvent;
protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    virtual Simple_task * genTask();
    virtual void sendTask(Simple_task *);
    virtual bool saveTask(Simple_task *);
    virtual void processResult(Simple_result *);
    virtual void pivotAround();

public:
    ~SimpleUser();
};

#endif /* SIMPLEUSER_H_ */
