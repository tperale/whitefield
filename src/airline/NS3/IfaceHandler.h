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
 * @brief       IfaceHandler Header
 *
 * @author      Rahul Jadhav <nyrahul@gmail.com>
 *
 * @}
 */

#ifndef _IFACEHANDLER_H_
#define _IFACEHANDLER_H_

#include "ns3/spectrum-phy.h"
#include <ns3/ptr.h>
#include <ns3/net-device.h>
#include <ns3/node-container.h>
#include <ns3/node.h>

/* #if PLC */
/* #include <ns3/plc-node.h> */
/* typedef std::vector<ns3::Ptr<ns3::PLC_Node> > PLC_NodeList; */
/* #endif */

extern "C" {
#include "commline/commline.h"
}

using namespace ns3;

class IFace: public ns3::Node {
public:
    virtual int init() { return 0; };
    virtual int init(ns3::Ptr<ns3::SpectrumChannel> channel) {return 0;};
    virtual int setParam(cl_param_t param, void* src, size_t len) {return 0;};
    virtual int sendPacket(msg_buf_t* mbuf) {return 0;};

    /* IFace() : Node() {}; */
};

#endif // _IFACEHANDLER_H_
