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

#ifndef _AIRLINEMANAGER_H_
#define _AIRLINEMANAGER_H_

#include <common.h>
#include <Nodeinfo.h>
#include <Config.h>

#include <ns3/node-container.h>
#include <ns3/core-module.h>

#include "IfaceHandler.h"

using namespace ns3;

class AirlineManager {
private:
    ifaceCtx_t g_ifctx;
    ifaceApi_t *iface;
    EventId m_sendEvent;

    void    msgrecvCallback(msg_buf_t *mbuf);
    int     phyInstall(NodeContainer &nodes);
    int     startIface();
    int     startNetwork(wf::Config &cfg);
    void    nodePos(NodeContainer const &nodes, uint16_t id, double &x, double &y, double &z);
    int     cmd_node_exec(uint16_t id, char *buf, int buflen);
    int     cmd_node_position(uint16_t id, char *buf, int buflen);
    int     cmd_set_node_position(uint16_t id, char *buf, int buflen);
    int     cmd_802154_set_short_addr(uint16_t id, char *buf, int buflen);
    int     cmd_802154_set_ext_addr(uint16_t id, char *buf, int buflen);
    int     cmd_802154_set_panid(uint16_t id, char *buf, int buflen);
    void    setPositionAllocator(NodeContainer &nodes);
    void    setNodeSpecificParam(NodeContainer &nodes);
    int     setAllNodesParam(NodeContainer &nodes);
    void    msgReader(void);
    void    ScheduleCommlineRX(void);

public:
    AirlineManager(wf::Config &cfg);
    ~AirlineManager();
};

#endif //_AIRLINEMANAGER_H_
