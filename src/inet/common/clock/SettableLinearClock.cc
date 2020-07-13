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

#include "inet/common/clock/SettableLinearClock.h"

namespace inet {

Define_Module(SettableLinearClock);

void SettableLinearClock::initialize()
{
    driftRate = 0.0;
    origin.simtime = simTime();
    simtime_t clock = par("timeShift");
    origin.clocktime = ClockTime::from(simTime() + clock);
    driftRate = par("driftRate").doubleValue() / 1e6;
}

clocktime_t SettableLinearClock::getClockTime() const
{
    simtime_t t = simTime();
    return origin.clocktime + ClockTime::from((t-origin.simtime) / (1 + driftRate));
}

void SettableLinearClock::scheduleClockEvent(clocktime_t t, cMessage *msg)
{
    simtime_t now = simTime();
    for (auto it = timers.begin(); it != timers.end(); ) {
        if (it->arrivalTime.simtime <= now)
            it = timers.erase(it);
        else {
            ASSERT(it->msg != msg);
            ++it;
        }
    }
    Timer timer;
    timer.module = getTargetModule();
    timer.msg = msg;
    timer.arrivalTime.clocktime = t;
    timer.arrivalTime.simtime = origin.simtime + (t - origin.clocktime).asSimTime() * (1 + driftRate);
    timer.module->scheduleAt(timer.arrivalTime.simtime, msg);
    timers.push_back(timer);
}

cMessage *SettableLinearClock::cancelClockEvent(cMessage *msg)
{
    simtime_t now = simTime();
    for (auto it = timers.begin(); it != timers.end(); ) {
        if (it->arrivalTime.simtime <= now || it->msg == msg)
            it = timers.erase(it);
        else
            ++it;
    }
    return getTargetModule()->cancelEvent(msg);
}

clocktime_t SettableLinearClock::getArrivalClockTime(cMessage *msg) const
{
    ASSERT(msg->isScheduled());
    for (auto timer : timers) {
        if (timer.msg == msg)
            return timer.arrivalTime.clocktime;
    }
    throw cRuntimeError("Message not found in Timer vector");
}

void SettableLinearClock::purgeTimers()
{
    simtime_t now = simTime();
    for (auto it = timers.begin(); it != timers.end(); ) {
        if (it->arrivalTime.simtime <= now)
            it = timers.erase(it);
        else
            ++it;
    }
}

void SettableLinearClock::rescheduleTimers()
{
    simtime_t now = simTime();
    for (auto it = timers.begin(); it != timers.end(); ) {
        if (it->arrivalTime.simtime <= now)
            it = timers.erase(it);
        else {
            simtime_t st = origin.simtime + (it->arrivalTime.clocktime - origin.clocktime).asSimTime() * (1 + driftRate);
            if (st < now)
                st = now;
            {
                cContextSwitcher tmp(it->module);
                it->module->cancelEvent(it->msg);
                it->module->scheduleAt(st, it->msg);
                it->arrivalTime.simtime = st;
            }
            ++it;
        }
    }
}

void SettableLinearClock::setDriftRate(double newDriftRate)
{
    driftRate = newDriftRate;
    EV_DEBUG << "set driftRate to " << driftRate << " at " << simTime() << endl;
    rescheduleTimers();
}

void SettableLinearClock::setClockTime(clocktime_t t)
{
    origin.simtime = simTime();
    origin.clocktime = t;
    EV_DEBUG << "set clock time to " << origin.clocktime << " at " << origin.simtime << endl;
    rescheduleTimers();
}

void SettableLinearClock::processCommand(const cXMLElement& node)
{
    if (!strcmp(node.getTagName(), "set-clock")) {
        if (const char *clockTimeStr = node.getAttribute("time")) {
            EV_DEBUG << "processCommand: set clock time to " << clockTimeStr << endl;
            clocktime_t t = ClockTime::parse(clockTimeStr);
            setClockTime(t);
        }
        if (const char *driftRateStr = node.getAttribute("driftRate")) {
            EV_DEBUG << "processCommand: set drift rate to " << driftRateStr << endl;
            double rate = strtod(driftRateStr, nullptr);
            setDriftRate(rate);
        }
    }
    else
        throw cRuntimeError("invalid command node for %s at %s", getClassName(), node.getSourceLocation());
}


} // namespace inet

