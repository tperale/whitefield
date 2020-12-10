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
    IFace* iface;
    EventId m_sendEvent;

    int     cmd_node_exec(uint16_t id, char *buf, int buflen);
    int     cmd_node_position(uint16_t id, char *buf, int buflen);
    int     cmd_set_node_position(uint16_t id, char *buf, int buflen);
    int     cmd_802154_set_short_addr(uint16_t id, char *buf, int buflen);
    int     cmd_802154_set_ext_addr(uint16_t id, char *buf, int buflen);
    int     cmd_802154_set_panid(uint16_t id, char *buf, int buflen);

    void    commMsgCallback(msg_buf_t *mbuf);
    void    commMsgReader(void);

    void    ScheduleCommlineRX(void);
    void    nodePos(NodeContainer const &nodes, uint16_t id, double &x, double &y, double &z);
    void    setNodeSpecificParam(NodeContainer &nodes);
    int     setAllNodesParam(NodeContainer &nodes);
    void    setPositionAllocator(NodeContainer &nodes);
    int     startNetwork(wf::Config &cfg);

public:
    AirlineManager(wf::Config &cfg);
    ~AirlineManager();
};

#endif //_AIRLINEMANAGER_H_
