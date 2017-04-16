#define	_AIRLINEMANAGER_CC_

#include <unistd.h>
#include <thread>

#include "AirlineManager.h"
#include "Airline.h"
extern "C" {
#include "commline/commline.h"
}

void AirlineManager::getAllNodeInfo(void)
{
	NodeContainer const & n = NodeContainer::GetGlobal (); 
	for (NodeContainer::Iterator i = n.Begin (); i != n.End (); ++i) 
	{ 
		Ptr<Node> node = *i; 
		//std::string name = Names::FindName (node);
		Ptr<MobilityModel> mob = node->GetObject<MobilityModel> (); 
		if (! mob) continue; // Strange, node has no mobility model installed. Skip. 

		Vector pos = mob->GetPosition (); 
		Ptr<LrWpanNetDevice> dev = node->GetDevice(0)->GetObject<LrWpanNetDevice>();
		std::cout << "Node " << node->GetId() << " is at (" << pos.x << ", " << pos.y << ", " << pos.z 
				  << ") shortaddr=" << dev->GetMac()->GetShortAddress()
				  //<< " ExtAddr:" << dev->GetMac()->GetExtendedAddress()
				  << " PanID:" << dev->GetMac()->GetPanId() 
				  << "\n"; 
	} 
}

void AirlineManager::commline_thread(void)
{
	uint8_t buf[sizeof(msg_buf_t) + COMMLINE_MAX_BUF];
	msg_buf_t *mbuf=(msg_buf_t*)buf;
	int slptime=1;

	INFO << "Commline Thread created\n";
	while(1)
	{
		usleep(slptime);

		if(CL_SUCCESS!=cl_recvfrom_q(MTYPE(AIRLINE,0), mbuf, sizeof(buf))) {
			break;
		}
		slptime=mbuf->len?1:1000;
#if 0
		{
			NodeContainer const & n = NodeContainer::GetGlobal (); 
			Ptr<Application> nodeApp = n.Get(2)->GetApplication(0);
			if(nodeApp) {
				Ptr<Airline> aline = DynamicCast<Airline> (nodeApp);
				aline->rxPacketFromStackline(buf, len);
			}
		}
#endif
	}
	INFO << "stopping commline_thread\n";
}

void AirlineManager::setMobilityModel(MobilityHelper & mobility)
{
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
			"MinX", DoubleValue(.0),
			"MinY", DoubleValue(.0),
			"DeltaX", DoubleValue(stod(CFG("fieldX"))),
			"DeltaY", DoubleValue(stod(CFG("fieldY"))),
			"GridWidth", UintegerValue(stoi(CFG("deploymentMode"))),
			"LayoutType", StringValue("RowFirst"));

	//TODO: In the future this could support different types of mobility models
}

int AirlineManager::startNetwork(wf::Config & cfg)
{
	NodeContainer nodes;
	nodes.Create (cfg.getNumberOfNodes());
	INFO << "Creating " << cfg.getNumberOfNodes() << " nodes..\n";
	SeedManager::SetSeed(0xbabe);

	MobilityHelper mobility;
	setMobilityModel(mobility);
	mobility.Install (nodes);

	NS_LOG_INFO ("Create channels.");
	LrWpanHelper lrWpanHelper;
	NetDeviceContainer devContainer = lrWpanHelper.Install(nodes);
	lrWpanHelper.AssociateToPan (devContainer, CFG_PANID);

	string ns3_capfile = CFG("NS3_captureFile");
	if(!ns3_capfile.empty()) {
		INFO << "NS3 Capture File:" << ns3_capfile << endl;
		lrWpanHelper.EnablePcapAll (ns3_capfile, true);
	}

	AirlineHelper airlineApp;
	ApplicationContainer apps = airlineApp.Install(nodes);
	apps.Start(Seconds(0.0));

	thread t1(commline_thread);
	t1.detach();
	Simulator::Run ();
	getAllNodeInfo();
	pause();
	Simulator::Destroy ();
	INFO << "Execution done\n";
	return SUCCESS;
}

AirlineManager::AirlineManager(wf::Config & cfg)
{
	startNetwork(cfg);
	INFO << "AirlineManager started" << endl;
}

AirlineManager::~AirlineManager() 
{
}
