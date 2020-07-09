#include "inet/common/base/ClockUsingModuleMixin.h"

namespace inet {

class ClockTestApp : public ClockUsingModuleMixin<cSimpleModule>
{
private:
        size_t idx = 0;
        std::vector<clocktime_t> timeVector;
    public:
       ClockTestApp() : ClockUsingModuleMixin() {}
    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override;
    protected:
        void doSchedule(clocktime_t t, cMessage *msg);
};

Define_Module(ClockTestApp);

void ClockTestApp::initialize(int stage)
{
    ClockUsingModuleMixin::initialize(stage);

    if (stage == INITSTAGE_LAST) {
        timeVector.push_back(1.1);
        timeVector.push_back(2.11);
        timeVector.push_back(2.89);
        timeVector.push_back(2.9999999);
        timeVector.push_back(3.11);
        idx = 0;
        auto msg = new cMessage("Timer");
        scheduleAt(SIMTIME_ZERO, msg);
    }
}

void ClockTestApp::doSchedule(clocktime_t ct, cMessage *msg)
{
    scheduleClockEvent(ct, msg);
    EV << "schedule to clock: " << ct << ", scheduled simtime: " << msg->getArrivalTime() << ", scheduled clock: " << getArrivalClockTime(msg) << endl;
}

void ClockTestApp::handleMessage(cMessage *msg)
{
    ASSERT(msg->isSelfMessage());
    EV << "arrived: " << simTime() << endl;
    if (idx < timeVector.size()) {
        doSchedule(timeVector[idx++], msg);
    }
    else {
        delete msg;
    }
}

}

