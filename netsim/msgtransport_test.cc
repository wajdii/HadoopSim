#include "netsim/msgtransport.h"
#include "gtest/gtest.h"
namespace HadoopNetSim {

class MsgTransportTestSink : public ns3::Application {
  public:
    ns3::Ptr<ns3::Socket> sock_listen_;
    ns3::Ptr<MsgTransport> mt_;
    ns3::Ptr<MsgInfo> msg_;

    MsgTransportTestSink(void) {
      this->sock_listen_ = NULL;
      this->mt_ = NULL;
      this->msg_ = NULL;
    }
    virtual ~MsgTransportTestSink(void) {}
    static ns3::TypeId GetTypeId(void) {
      static ns3::TypeId tid = ns3::TypeId("HadoopNetSim::MsgTransportTestSink")
                               .SetParent<ns3::Application>()
                               .AddConstructor<MsgTransportTestSink>();
      return tid;
    }
    
  private:
    void StartApplication() {
      this->sock_listen_ = ns3::Socket::CreateSocket(this->GetNode(), ns3::TcpSocketFactory::GetTypeId());
      ns3::InetSocketAddress addr = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), 80);
      this->sock_listen_->Bind(addr);
      this->sock_listen_->Listen();
      this->sock_listen_->ShutdownSend();
      this->sock_listen_->SetAcceptCallback(ns3::MakeNullCallback<bool, ns3::Ptr<ns3::Socket>, const ns3::Address&>(),
                                            ns3::MakeCallback(&MsgTransportTestSink::HandleAccept, this));
    }
    void StopApplication() {}
    
    void HandleAccept(ns3::Ptr<ns3::Socket> sock, const ns3::Address& from) {
      this->mt_ = ns3::Create<MsgTransport>(sock);this->mt_->Ref();
      this->mt_->set_recv_cb(ns3::MakeCallback(&MsgTransportTestSink::Recv, this));
    }
    void Recv(ns3::Ptr<MsgTransport>, ns3::Ptr<MsgInfo> msg) {
      this->msg_ = msg;
    }
    
    DISALLOW_COPY_AND_ASSIGN(MsgTransportTestSink);
};

class MsgTransportTestSource : public ns3::Application {
  public:
    ns3::Ipv4Address peer_;
    ns3::Ptr<MsgTransport> mt_;
    ns3::Ptr<MsgInfo> msg_;
    int send_called_;
    int transmit_cb_called_;

    MsgTransportTestSource(void) {
      this->mt_ = NULL;
      this->send_called_ = 0;
      this->transmit_cb_called_ = 0;
    }
    virtual ~MsgTransportTestSource(void) {}
    static ns3::TypeId GetTypeId(void) {
      static ns3::TypeId tid = ns3::TypeId("HadoopNetSim::MsgTransportTestSource")
                               .SetParent<ns3::Application>()
                               .AddConstructor<MsgTransportTestSource>();
      return tid;
    }
    
  private:
    void StartApplication() {
      ns3::Ptr<ns3::Socket> sock = ns3::Socket::CreateSocket(this->GetNode(), ns3::TcpSocketFactory::GetTypeId());
      sock->Bind();
      sock->Connect(ns3::InetSocketAddress(ns3::Ipv4Address("192.168.72.1"), 80));
      this->msg_ = ns3::Create<MsgInfo>();
      this->msg_->set_id(50); this->msg_->set_size(64*1<<20); this->msg_->set_src("source"); this->msg_->set_dst("sink");
      this->msg_->set_cb(ns3::MakeCallback(&MsgTransportTestSource::TransmitCallback, this));
      this->mt_ = ns3::Create<MsgTransport>(sock, false);
      this->mt_->set_send_cb(ns3::MakeCallback(&MsgTransportTestSource::HandleSend, this));
      this->mt_->Send(this->msg_);

      /*
      ns3::Ptr<ns3::Socket> sock2 = ns3::Socket::CreateSocket(this->GetNode(), ns3::TcpSocketFactory::GetTypeId());
      sock2->Bind();
      sock2->Connect(ns3::InetSocketAddress(ns3::Ipv4Address("192.168.72.1"), 80));
      ns3::Ptr<MsgInfo> msg2 = ns3::Create<MsgInfo>();
      msg2->set_id(51); msg2->set_size(64*1<<20); msg2->set_src("source"); msg2->set_dst("sink");
      //msg2->set_cb(ns3::MakeCallback(&MsgTransportTestSource::TransmitCallback, this));
      ns3::Ptr<MsgTransport> mt2 = ns3::Create<MsgTransport>(sock2, false);
      mt2->Send(msg2);
      mt2->Ref();
      ns3::Ptr<ns3::Socket> sock3 = ns3::Socket::CreateSocket(this->GetNode(), ns3::TcpSocketFactory::GetTypeId());
      sock3->Bind();
      sock3->Connect(ns3::InetSocketAddress(ns3::Ipv4Address("192.168.72.1"), 80));
      ns3::Ptr<MsgInfo> msg3 = ns3::Create<MsgInfo>();
      msg3->set_id(52); msg3->set_size(64*1<<20);
      //msg3->set_cb(ns3::MakeCallback(&MsgTransportTestSource::TransmitCallback, this));
      ns3::Ptr<MsgTransport> mt3 = ns3::Create<MsgTransport>(sock3, false);
      mt3->Send(msg3);msg3->Ref();
      mt3->Ref();
      */
    }
    void StopApplication() {}
    
    void HandleSend(ns3::Ptr<MsgTransport>, ns3::Ptr<MsgInfo>) {
      ++this->send_called_;
    }
    void TransmitCallback(ns3::Ptr<MsgInfo>) {
      ++this->transmit_cb_called_;
    }
    
    DISALLOW_COPY_AND_ASSIGN(MsgTransportTestSource);
};

TEST(NetSimTest, MsgTransport) {
  EXPECT_EXIT({
    ns3::NodeContainer nodes; nodes.Create(2);
    ns3::PointToPointHelper ptp;
    ptp.SetDeviceAttribute("DataRate", ns3::StringValue("1Gbps"));
    ptp.SetDeviceAttribute("Mtu", ns3::UintegerValue(9000));
    ptp.SetChannelAttribute("Delay", ns3::TimeValue(ns3::NanoSeconds(6560)));
    ns3::NetDeviceContainer devices = ptp.Install(nodes);
    ns3::Config::SetDefault("ns3::TcpSocket::SegmentSize", ns3::UintegerValue(8500));
    ns3::InternetStackHelper inetstack; inetstack.Install(nodes);
    ns3::Ipv4AddressHelper ip4addr; ip4addr.SetBase("192.168.72.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer ifs = ip4addr.Assign(devices);
    
    ns3::Ptr<MsgTransportTestSink> sink = ns3::CreateObject<MsgTransportTestSink>();
    nodes.Get(0)->AddApplication(sink);
    ns3::Ptr<MsgTransportTestSource> source = ns3::CreateObject<MsgTransportTestSource>();
    sink->SetStartTime(ns3::Seconds(0));
    nodes.Get(1)->AddApplication(source);
    source->SetStartTime(ns3::Seconds(1));
    
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    if (ns3::PeekPointer(sink->msg_) != ns3::PeekPointer(source->msg_)) ::exit(1);
    if (source->send_called_ != 1) ::exit(2);
    if (source->transmit_cb_called_ != 1) ::exit(3);
    ::exit(0);
  }, ::testing::ExitedWithCode(0), "");
}

};//namespace HadoopNetSim