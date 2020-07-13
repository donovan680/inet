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

#include "inet/common/clock/SettableGranularityClock.h"

namespace inet {

Define_Module(SettableGranularityClock);

void SettableGranularityClock::initialize()
{
    origin.simtime = simTime();
    origin.clocktime = ClockTime::from(origin.simtime);
    driftRate = 0.0;
}

void SettableGranularityClock::handleParameterChange(const char *name)
{
    if (name == nullptr || !strcmp(name, "origin")) {
        setClockTime(par("origin"));
    }
    if (name == nullptr || !strcmp(name, "driftRate")) {
        setDriftRate(par("driftRate").doubleValue() / 1e6);
    }
    if (name == nullptr || !strcmp(name, "granularity")) {
        setGranularity(par("granularity"));
    }
}


clocktime_t SettableGranularityClock::getClockTime() const
{
    simtime_t t = simTime();
    return fromSimTime(t);
}

void SettableGranularityClock::scheduleClockEvent(clocktime_t t, cMessage *msg)
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
    timer.arrivalTime.simtime = toSimTime(t);
    timer.module->scheduleAt(timer.arrivalTime.simtime, msg);
    timers.push_back(timer);
}

cMessage *SettableGranularityClock::cancelClockEvent(cMessage *msg)
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

clocktime_t SettableGranularityClock::getArrivalClockTime(cMessage *msg) const
{
    return fromSimTime(msg->getArrivalTime()); // note: imprecision due to conversion to simtime and forth
}

void SettableGranularityClock::purgeTimers()
{
    simtime_t now = simTime();
    for (auto it = timers.begin(); it != timers.end(); ) {
        if (it->arrivalTime.simtime <= now)
            it = timers.erase(it);
        else
            ++it;
    }
}

void SettableGranularityClock::rescheduleTimers()
{
    simtime_t now = simTime();
    for (auto it = timers.begin(); it != timers.end(); ) {
        if (it->arrivalTime.simtime <= now)
            it = timers.erase(it);
        else {
            simtime_t st = toSimTime(it->arrivalTime.clocktime);
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

void SettableGranularityClock::setDriftRate(double newDriftRate)
{
    driftRate = newDriftRate;
    rescheduleTimers();
}

void SettableGranularityClock::setClockTime(clocktime_t t)
{
    origin.simtime = simTime();
    origin.clocktime = t;
    rescheduleTimers();
}

void SettableGranularityClock::setGranularity(clocktime_t t)
{
    granularity = t;
    if (granularity < CLOCKTIME_ZERO)
        throw cRuntimeError("incorrect granularity value: %s, granularity must be 0 or positive value", granularity.str().c_str());
    if (granularity == CLOCKTIME_ZERO)
        granularity.setRaw(1);
}

clocktime_t SettableGranularityClock::fromSimTime(simtime_t t) const
{
    auto granularityRaw = granularity.raw();
    clocktime_t clock = ClockTime::from((t - origin.simtime) / (1.0 + driftRate));
    clock.setRaw((clock.raw() / granularityRaw) * granularityRaw);
    clock += origin.clocktime;
    return clock;
}

simtime_t SettableGranularityClock::toSimTime(clocktime_t clock) const
{
    auto granularityRaw = granularity.raw();
    clock -= origin.clocktime;
    clock.setRaw(((clock.raw() + granularityRaw - 1) / granularityRaw) * granularityRaw);
    return origin.simtime + clock.asSimTime() * (1.0 + driftRate);
}


} // namespace inet

