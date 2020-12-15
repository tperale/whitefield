/*
 * Copyright (C) 2017 Rahul Jadhav <nyrahul@gmail.com>
 *
 * This file is subject to the terms and conditions of the GNU
 * General Public License v2. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     airline
 * @{
 *
 * @file
 * @brief       NS3 specific airline module for managing virtual node set.
 *
 * @author      Rahul Jadhav <nyrahul@gmail.com>
 *
 * @}
 */

#include "IFaceContainer.h"
#include "LrwpanContainer.h"
#include "commline/commline.h"
#define	_AIRLINEMANAGER_CC_

#include <map>
#include <string>
#include <sstream>
#include <iostream>

#include <ns3/single-model-spectrum-channel.h>
#include <ns3/mobility-module.h>
#include <ns3/spectrum-value.h>

#include "AirlineManager.h"
#include "Airline.h"
#include "Command.h"
#include "mac_stats.h"
#include "LrwpanIface.h"

int getNodeConfigVal(int id, char *key, char *val, int vallen)
{
    wf::Nodeinfo *ni=NULL;
    ni=WF_config.get_node_info(id);
    if (!ni) {
        snprintf(val, vallen, "cudnot get nodeinfo id=%d", id);
        CERROR << "Unable to get node config\n";
        return 0;
    }
    if (!strcmp(key, "nodeexec")) {
        return snprintf(val, vallen, "%s", 
                (char *)ni->getNodeExecutable().c_str());
    }
    snprintf(val, vallen, "unknown key [%s]", key);
    CERROR << "Unknown key " << key << "\n";
    return 0;
}

int AirlineManager::cmd_node_exec(uint16_t id, char *buf, int buflen)
{
    return getNodeConfigVal(id, (char *)"nodeexec", buf, buflen);
}

int AirlineManager::cmd_node_position(uint16_t id, char *buf, int buflen)
{
    int n=0;
    ofstream of;
    NodeContainer const & nodes = NodeContainer::GetGlobal (); 
    if(buf[0]) {
        of.open(buf);
        if(!of.is_open()) {
            char tmpbuf[256];
            snprintf(tmpbuf, sizeof(tmpbuf), "%s", buf);
            return snprintf(buf, buflen, "could not open file %s", tmpbuf);
        } else {
            n = snprintf(buf, buflen, "SUCCESS");
        }
    }
    for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i) 
    { 
        Ptr<Node> node = *i; 
        Ptr<MobilityModel> mob = node->GetObject<MobilityModel> (); 
        if (! mob) continue;

        Vector pos = mob->GetPosition (); 
        if(id == 0xffff || id == node->GetId()) {
            if(of.is_open()) {
                of << "Node " << node->GetId() << " Location= " << pos.x << " " << pos.y << " " << pos.z << "\n"; 
            } else {
                n += snprintf(buf+n, buflen-n, "%d loc= %.2f %.2f %.2f\n", node->GetId(), pos.x, pos.y, pos.z);
                if(n > (buflen-50)) {
                    n += snprintf(buf+n, buflen-n, "[TRUNC]");
                    break;
                }
            }
        }
    }
    of.close();
    return n;
}

int AirlineManager::cmd_802154_set_ext_addr(uint16_t id, char *buf, int buflen)
{
    int ret = nodes->setParam(id, CL_IEEE_802_15_4_EXT_ADDRESS, buf, buflen);
    if (ret == SUCCESS) {
        return snprintf(buf, buflen, "SUCCESS");
    }
    return snprintf(buf, buflen, "FAILURE");
}

int AirlineManager::cmd_set_node_position(uint16_t id, char *buf, int buflen)
{
    char *ptr, *saveptr;
    double x, y, z=0;
    NodeContainer const & nodes = NodeContainer::GetGlobal (); 
    int numNodes = stoi(CFG("numOfNodes"));

    if(!IN_RANGE(id, 0, numNodes)) {
            return snprintf(buf, buflen,
            "NodeID mandatory for setting node pos id=%d", id);
    }
    ptr = strtok_r(buf, " ", &saveptr);
    if(!ptr) return snprintf(buf, buflen, "invalid loc format! No x pos!");
    x = stod(ptr);
    ptr = strtok_r(NULL, " ", &saveptr);
    if(!ptr) return snprintf(buf, buflen, "invalid loc format! No y pos!");
    y = stod(ptr);
    ptr = strtok_r(NULL, " ", &saveptr);
    if(ptr) z = stod(ptr);

    Ptr<MobilityModel> mob = nodes.Get(id)->GetObject<MobilityModel>();
    Vector m_position = mob->GetPosition();
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
    mob->SetPosition(m_position);
    return snprintf(buf, buflen, "SUCCESS");
}

void AirlineManager::commMsgCallback(msg_buf_t *mbuf)
{
    NodeContainer const & n = NodeContainer::GetGlobal (); 
    int numNodes = stoi(CFG("numOfNodes"));

    if(mbuf->flags == MBUF_IS_CMD) {
        // TODO Handle CMD are commands written to the shell I need to change the location the cmd are handled to IFace
        if(0) {}
            HANDLE_CMD(mbuf, cmd_node_exec)
            HANDLE_CMD(mbuf, cmd_node_position)
            HANDLE_CMD(mbuf, cmd_set_node_position)
            HANDLE_CMD(mbuf, cmd_802154_set_ext_addr)
        else {
            al_handle_cmd(mbuf);
        }
        cl_sendto_q(MTYPE(MONITOR, CL_MGR_ID), mbuf, mbuf->len+sizeof(msg_buf_t));
        /* TODO MBUF_DO_NOT_RESPOND is not used anywhere */
        /* if(!(mbuf->flags & MBUF_DO_NOT_RESPOND)) { */
        /* } */
        return;
    }

    if(!IN_RANGE(mbuf->src_id, 0, numNodes)) {
        CERROR << "rcvd src id=" << mbuf->src_id << " out of range!!\n";
        return;
    } else if(mbuf->dst_id == CL_DSTID_MACHDR_PRESENT) {
        if(CFG_INT("macHeaderAdd", 1)) {
            CERROR << "rcvd a packet from stackline with DSTID_MACHDR_PRESENT set but config file does not have macHeaderAdd=0\n";
            CERROR << "If you are using openthread, please set macHeaderAdd=0 to prevent Airline from adding its own mac hdr\n";
            return;
        }
    }

    switch (mbuf->flags) {
    case MBUF_IS_SEND:
        nodes->sendPacket(mbuf->src_id, mbuf);
        wf::Macstats::set_stats(AL_TX, mbuf);
        break;
    case MBUF_IS_PARAM:
        nodes->setParam(mbuf->src_id, (cl_param_t) mbuf->param, mbuf->buf, mbuf->len);
        break;
    default:
        break;
    }
}

void AirlineManager::commMsgReader(void)
{
    DEFINE_MBUF(mbuf);
    while(1) {
        cl_recvfrom_q(MTYPE(AIRLINE,CL_MGR_ID), mbuf, sizeof(mbuf_buf), CL_FLAG_NOWAIT);
        if(mbuf->len) {
            commMsgCallback(mbuf);
            usleep(1);
        } else {
            break;
        }
    }
    ScheduleCommlineRX();
}

void AirlineManager::ScheduleCommlineRX(void)
{
    m_sendEvent = Simulator::Schedule (Seconds(0.001), &AirlineManager::commMsgReader, this);
}

void AirlineManager::nodePos(IFaceContainer* nodes, uint16_t id, double & x, double & y, double & z)
{
    MobilityHelper mob;
    Ptr<ListPositionAllocator> positionAlloc = 
    CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (x, y, z));
    mob.SetPositionAllocator (positionAlloc);
    mob.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mob.Install(nodes->Get(id));
}

void AirlineManager::setNodeSpecificParam(IFaceContainer* nodes)
{
    uint8_t is_set=0;
    double x, y, z;
    wf::Nodeinfo *ni=NULL;
    string txpower, deftxpower = CFG("txPower");

    for(int i = 0; i < ((int) nodes->GetN()); i++) {
        ni = WF_config.get_node_info(i);
        if(!ni) {
            CERROR << "GetN doesnt match nodes stored in config!!\n";
            return;
        }
        ni->getNodePosition(is_set, x, y, z);
        if(is_set) {
            nodePos(nodes, i, x, y, z);
        }
        if(ni->getPromisMode()) {
            nodes->setParam(i, CL_IEEE_802_15_4_PROMISCUOUS, (void*) NULL, 0);
        }
        txpower = ni->getkv("txPower");
        if (txpower.empty())
            txpower = deftxpower;

        if (!txpower.empty()) {
            double dtxpower = stod(txpower);
            nodes->setParam(i, CL_IEEE_802_15_4_TX_POWER, (void*) &dtxpower, sizeof(double));
        }
    }
}

void AirlineManager::setPositionAllocator(IFaceContainer* nodes)
{
    MobilityHelper mobility;
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    //TODO: In the future this could support different types of mobility models

    if(CFG("topologyType") == "grid") {
        int gw=stoi(CFG("gridWidth"));
        CINFO << "Using GridPositionAllocator\n";
        mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
            "MinX", DoubleValue(.0),
            "MinY", DoubleValue(.0),
            "DeltaX", DoubleValue(stod(CFG("fieldX"))/gw),
            "DeltaY", DoubleValue(stod(CFG("fieldY"))/gw),
            "GridWidth", UintegerValue(gw),
            "LayoutType", StringValue("RowFirst"));
    } else if(CFG("topologyType") == "randrect") {
        char x_buf[128], y_buf[128];
        snprintf(x_buf, sizeof(x_buf), "ns3::UniformRandomVariable[Min=0.0|Max=%s]", CFG("fieldX").c_str());
        snprintf(y_buf, sizeof(y_buf), "ns3::UniformRandomVariable[Min=0.0|Max=%s]", CFG("fieldY").c_str());
        CINFO << "Using RandomRectanglePositionAllocator\n";
        mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator", "X", StringValue(x_buf), "Y", StringValue(y_buf));
    } else {
        CERROR << "Unknown topologyType: " << CFG("topologyType") << " in cfg\n";
        throw FAILURE;
    }

    for (NodeContainer::Iterator i = nodes->Begin (); i != nodes->End (); ++i) {
        mobility.Install(*i);
    }
}

int AirlineManager::startNetwork(wf::Config & cfg)
{
    try {
        GlobalValue::Bind ("ChecksumEnabled", 
        BooleanValue (CFG_INT("macChecksumEnabled", 1)));
        GlobalValue::Bind ("SimulatorImplementationType", 
        StringValue ("ns3::RealtimeSimulatorImpl"));

        wf::Macstats::clear();

        nodes->Create(cfg.getNumberOfNodes());
        CINFO << "Creating " << cfg.getNumberOfNodes() << " nodes..\n";
        SeedManager::SetSeed(stoi(CFG("randSeed", "0xbabe"), nullptr, 0));

        setPositionAllocator(nodes);

        if (nodes->setup() != SUCCESS) {
            return FAILURE;
        }

        setNodeSpecificParam(nodes);

        AirlineHelper airlineApp;
        ApplicationContainer apps = airlineApp.Install(nodes);
        apps.Start(Seconds(0.0));

        ScheduleCommlineRX();
        CINFO << "NS3 Simulator::Run initiated..." << endl;
        fflush(stdout);
        Simulator::Run ();
        pause();
        Simulator::Destroy ();
    } catch (int e) {
        CERROR << "Configuration failed" << endl;
        return FAILURE;
    }

    return SUCCESS;
}

AirlineManager::AirlineManager(wf::Config & cfg)
{
    m_sendEvent = EventId ();

    string phy = CFG("PHY");
    if (!stricmp(phy, "plc")) {
        // TODO PLC
    } else if (!stricmp(phy, "lr-wpan")) {
        nodes = new LrwpanContainer();
    } else {
        nodes = new LrwpanContainer();
        INFO("NO PHY parameter\n");
    }

    startNetwork(cfg);
    INFO("AirlineManager started\n");
}

AirlineManager::~AirlineManager() 
{
    Simulator::Cancel (m_sendEvent);
}
