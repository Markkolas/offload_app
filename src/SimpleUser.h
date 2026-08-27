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

#include "simple_m.h"

class SimpleUser : public omnetpp::cSimpleModule  {
private:
    const int MIN_TASK_S = 100;
    const int MAX_TASK_S = 10000;
    omnetpp::cArray task_buffer{"tx_buff"};

    int countMsg = 0;
    int lastResult = 0;

    Simple_task * getTaskFromId(int Id);
protected:
    virtual void initialize() override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

    omnetpp::cMessage *timerEvent = nullptr;

public:
    ~SimpleUser();
};

#endif /* SIMPLEUSER_H_ */
