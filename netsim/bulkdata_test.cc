#include "netsim/netsim.h"
#include <array>
#include <unordered_map>
#include "gtest/gtest.h"
namespace HadoopNetSim {

/*
TOPOLOGY
star or rackrow-6-4

EVENTS after ready
2 seconds
  a Server send 2 x 49 DataRequest (256B) to all other servers
  reply DataResponse (64MB)
*/

const std::array <std::string, 50> Server = {
    "server-1495.novalocal",
    "server-1445.novalocal",
    "server-1494.novalocal",
    "server-1488.novalocal",
    "server-1469.novalocal",
    "server-1493.novalocal",
    "server-1451.novalocal",
    "server-1456.novalocal",
    "server-1446.novalocal",
    "server-1453.novalocal",
    "server-1490.novalocal",
    "server-1483.novalocal",
    "server-1463.novalocal",
    "server-1497.novalocal",
    "server-1498.novalocal",
    "server-1444.novalocal",
    "server-1473.novalocal",
    "server-1468.novalocal",
    "server-1485.novalocal",
    "server-1480.novalocal",
    "server-1481.novalocal",
    "server-1496.novalocal",
    "server-1476.novalocal",
    "server-1486.novalocal",
    "server-1448.novalocal",
    "server-1455.novalocal",
    "server-1471.novalocal",
    "server-1492.novalocal",
    "server-1449.novalocal",
    "server-1474.novalocal",
    "server-1447.novalocal",
    "server-1457.novalocal",
    "server-1467.novalocal",
    "server-1470.novalocal",
    "server-1491.novalocal",
    "server-1450.novalocal",
    "server-1479.novalocal",
    "server-1477.novalocal",
    "server-1452.novalocal",
    "server-1489.novalocal",
    "server-1472.novalocal",
    "server-1484.novalocal",
    "server-1460.novalocal",
    "server-1464.novalocal",
    "server-1482.novalocal",
    "server-1465.novalocal",
    "server-1478.novalocal",
    "server-1475.novalocal",
    "server-1487.novalocal",
    "server-1461.novalocal"
};

class BulkDataTestRunner {
  public:
    BulkDataTestRunner(NetSim* netsim) {
      this->netsim_ = netsim;
      this->netsim_->set_ready_cb(ns3::MakeCallback(&BulkDataTestRunner::Ready, this));
      this->userobj_ = this;
      this->lastID = 0;
    }
    void Verify() {
      std::unordered_map<MsgType,ns3::Ptr<ns3::MinMaxAvgTotalCalculator<double>>,std::hash<int>> calcs;
      calcs[kMTDataRequest] = ns3::Create<ns3::MinMaxAvgTotalCalculator<double>>();
      calcs[kMTDataResponse] = ns3::Create<ns3::MinMaxAvgTotalCalculator<double>>();
      for (std::unordered_map<MsgId,ns3::Ptr<MsgInfo>>::const_iterator it = this->received_.cbegin(); it != this->received_.cend(); ++it) {
        ns3::Ptr<MsgInfo> msg = it->second;
        calcs[msg->type()]->Update((msg->finish() - msg->start()).GetSeconds());
      }
      int sentDataRequest = 0, sentDataResponse = 0;
      for (std::unordered_map<MsgId,MsgType>::const_iterator it = this->sent_.cbegin(); it != this->sent_.cend(); ++it) {
        if (it->second == kMTDataRequest) ++sentDataRequest;
        else if (it->second == kMTDataResponse) ++sentDataResponse;
      }
      printf("counts %ld %ld %d %d\n", calcs[kMTDataRequest]->getCount(), calcs[kMTDataResponse]->getCount(), sentDataRequest, sentDataResponse);
      assert(calcs[kMTDataRequest]->getCount() == 98);
      assert(calcs[kMTDataResponse]->getCount() == 98);
      printf("DataRequest %f,%f,%f\n", calcs[kMTDataRequest]->getMin(), calcs[kMTDataRequest]->getMean(), calcs[kMTDataRequest]->getMax());
      printf("DataResponse %f,%f,%f\n", calcs[kMTDataResponse]->getMin(), calcs[kMTDataResponse]->getMean(), calcs[kMTDataResponse]->getMax());
    }

  private:
    void* userobj_;
    NetSim* netsim_;
    std::unordered_map<MsgId,MsgType> sent_;
    std::unordered_map<MsgId,ns3::Ptr<MsgInfo>> received_;
    MsgId lastID;

    void Ready(NetSim*) {
      ns3::Simulator::Schedule(ns3::Seconds(2.0), &BulkDataTestRunner::DataRequestAll, this);
    }
    void DataRequestAll() {
      MsgId id;
      for (size_t i = 1; i < Server.size(); ++i) {
        id = this->netsim_->DataRequest(Server[0], Server[i], 1<<8, ns3::MakeCallback(&BulkDataTestRunner::DataResponse, this), this->userobj_);
        assert(id != MsgId_invalid);
        this->sent_[id] = kMTDataRequest;

        id = this->netsim_->DataRequest(Server[0], Server[i], 1<<8, ns3::MakeCallback(&BulkDataTestRunner::DataResponse, this), this->userobj_);
        assert(id != MsgId_invalid);
        this->sent_[id] = kMTDataRequest;

        if (i == Server.size() - 1) {
          lastID = id;
        }
      }
    }
    void DataResponse(ns3::Ptr<MsgInfo> request_msg) {
      assert(this->sent_[request_msg->id()] == kMTDataRequest);
      assert(request_msg->userobj() == this->userobj_);
      assert(this->received_.count(request_msg->id()) == 0);
      this->received_[request_msg->id()] = request_msg;
      MsgId id = this->netsim_->DataResponse(request_msg->id(), request_msg->dst(), request_msg->src(), 1<<26, ns3::MakeCallback(&BulkDataTestRunner::DataFinish, this), this->userobj_);
      assert(id != MsgId_invalid);
      this->sent_[id] = kMTDataResponse;
      if (lastID == request_msg->id()) {
        lastID = id;
      }
    }
    void DataFinish(ns3::Ptr<MsgInfo> response_msg) {
      assert(this->sent_[response_msg->id()] == kMTDataResponse);
      assert(response_msg->userobj() == this->userobj_);
      assert(this->received_.count(response_msg->id()) == 0);
      this->received_[response_msg->id()] = response_msg;
      if (lastID == response_msg->id()) {
        ns3::Simulator::Schedule(ns3::Simulator::Now(), &ns3::Simulator::Stop);
      }
    }
    DISALLOW_COPY_AND_ASSIGN(BulkDataTestRunner);
};

TEST(NetSimTest, BulkData) {
  Topology topology;
  topology.Load("examples/HadoopSim/bench-trace/star.nettopo");
//topology.Load("examples/HadoopSim/bench-trace/rackrow-6-4.nettopo");
  std::unordered_set<HostName> managers; managers.insert("manager");
  ASSERT_EQ(52, topology.nodes().size());

  EXPECT_EXIT({
    NetSim netsim;
    netsim.BuildTopology(topology);
    netsim.InstallApps(managers);

    BulkDataTestRunner runner(&netsim);
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
    runner.Verify();
    ::exit(0);
  }, ::testing::ExitedWithCode(0), "");
}

};//namespace HadoopNetSim
