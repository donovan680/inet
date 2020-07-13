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

#ifndef __INET_SettableLinearClock_H
#define __INET_SettableLinearClock_H

#include "inet/common/clock/base/ClockBase.h"
#include "inet/common/clock/contract/IClock.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

/**
 * Models a clock with a constant clock drift rate.
 */
class INET_API SettableLinearClock : public ClockBase, public IClock, public IScriptable
{
  private:
    struct TimePair {
        simtime_t simtime;
        clocktime_t clocktime;
    };
    struct Timer {
        cSimpleModule *module;
        cMessage *msg;
        TimePair arrivalTime;
    };
    TimePair origin;
    double driftRate;
    std::vector<Timer> timers;

  protected:
    void purgeTimers();
    void rescheduleTimers();

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual void initialize() override;

    /**
     * Return the current time.
     */
    virtual clocktime_t getClockTime() const override;

    /**
     * Schedule an event to be delivered to the context module at the given time.
     */
    virtual void scheduleClockEvent(clocktime_t t, cMessage *msg) override;

    /**
     * Cancels an event.
     */
    virtual cMessage *cancelClockEvent(cMessage *msg) override;

    /**
     * Returns the arrival time of a message scheduled via scheduleClockEvent().
     */
    virtual clocktime_t getArrivalClockTime(cMessage *msg) const override;

    virtual void setDriftRate(double newDriftRate);

    virtual void setClockTime(clocktime_t t);
};

} // namespace inet

#endif // ifndef __INET_SettableLinearClock_H

