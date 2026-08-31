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

#import <omnetpp.h>

#import "simple_m.h"

class SimpleServer : public omnetpp::cSimpleModule {
private:
    const int CPU_CYCLES = 300; //MHz
    const int N_CORES = 8;
    const int QUEUE_SIZE = 100;

    int fullq_events = 0;

    omnetpp::cMessage *procEvent = nullptr; // May be a good idea to use smart pointers for this

    omnetpp::cQueue queue_buff{"server queue"};

    omnetpp::simsignal_t qLengthSig, qFullEvent;
protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;
    omnetpp::simtime_t processTask(Simple_task *);
    bool saveTask(Simple_task *);
    void sendResult(Simple_task *);
};

#endif /* SIMPLESERVER_H_ */
