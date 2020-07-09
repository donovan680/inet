//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/clock/GranularityClock.h"

namespace inet {

Define_Module(GranularityClock);

void GranularityClock::initialize()
{
    timeShift = par("timeShift");
    granularity = par("granularity");
}

clocktime_t GranularityClock::fromSimTime(simtime_t t) const
{
    auto granularityRaw = granularity.raw();
    return ClockTime::from(SimTime().setRaw((t-timeShift).raw() / granularityRaw) * granularityRaw);
}

simtime_t GranularityClock::toSimTime(clocktime_t clock) const
{
    auto granularityRaw = granularity.raw();
    return SimTime().setRaw(((clock.raw() + granularityRaw - 1) / granularityRaw) * granularityRaw) + timeShift;
}

clocktime_t GranularityClock::getArrivalClockTime(cMessage *msg) const
{
    return fromSimTime(msg->getArrivalTime()); // note: imprecision due to conversion to simtime and forth
}

} // namespace inet

