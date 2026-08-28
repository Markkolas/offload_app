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

#ifndef DYNDELAYCH_H_
#define DYNDELAYCH_H_

#include <omnetpp.h>

// I only create this class to change processMessage implementation to read
// delay dynamically

namespace omnetpp{

class DynDelayCh : public cDelayChannel{
public:
    virtual Result processMessage(cMessage *msg, const SendOptions& options, simtime_t t) override;
};

} //namespace omnetpp



#endif /* DYNDELAYCH_H_ */
