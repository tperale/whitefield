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
 * @brief       Interface Handler for NS3
 *
 * @author      Rahul Jadhav <nyrahul@gmail.com>
 *
 * @}
 */

#include <LrwpanIface.h>

#include "commline/commline.h"
#include <cstdio>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/mobility-module.h>
#include <ns3/spectrum-value.h>

#include <PropagationModel.h>
#include <common.h>
#include <Nodeinfo.h>
#include <Config.h>
#include <IfaceHandler.h>

static uint8_t wf_ack_status(LrWpanMcpsDataConfirmStatus status)
{
    switch(status) {
        case IEEE_802_15_4_SUCCESS:
            return WF_STATUS_ACK_OK;
        case IEEE_802_15_4_NO_ACK:
            return WF_STATUS_NO_ACK;
        case IEEE_802_15_4_TRANSACTION_OVERFLOW:
        case IEEE_802_15_4_TRANSACTION_EXPIRED:
        case IEEE_802_15_4_CHANNEL_ACCESS_FAILURE:
            return WF_STATUS_ERR;       //can retry later
        case IEEE_802_15_4_INVALID_GTS:
        case IEEE_802_15_4_COUNTER_ERROR:
        case IEEE_802_15_4_FRAME_TOO_LONG:
        case IEEE_802_15_4_UNAVAILABLE_KEY:
        case IEEE_802_15_4_UNSUPPORTED_SECURITY:
        case IEEE_802_15_4_INVALID_PARAMETER:
        default:
            return WF_STATUS_FATAL;
    }
}

static uint16_t addr2id(const Mac16Address addr)
{
    uint16_t id=0;
    uint8_t str[2], *ptr=(uint8_t *)&id;
    addr.CopyTo(str);
    ptr[1] = str[0];
    ptr[0] = str[1];
    return id;
}

void LrwpanIface::txConfirm(int id, McpsDataRequestParams* req_params, uint8_t* retries, ns3::McpsDataConfirmParams params)
{
    CINFO << "[" << id << "] LrWpanMcpsDataConfirmStatus = " << params.m_status << " with " << ((int) *retries) << " retries" << endl;
    fflush(stdout);
    uint16_t dst_id = addr2id(req_params->m_dstAddr);

    if(dst_id == 0xffff) {
        return;
    }
#if 0
    INFO << "Sending ACK status" 
         << " src=" << id << " dst=" << dst_id
         << " status=" << params.m_status
         << " retries=" << (int)params.m_retries
         << " pktSize(inc mac-hdr)=" << params.m_pktSz << "\n";
    fflush(stdout);
#endif
    DEFINE_MBUF(mbuf);

    *retries = *retries + 1;
    uint8_t status = wf_ack_status(params.m_status);

    SendAckToStackline(id, dst_id, status, 1);
}

void LrwpanIface::rxIndication(int id, ns3::Ptr<ns3::LrWpanNetDevice> dev, ns3::McpsDataIndicationParams params, ns3::Ptr<ns3::Packet> p)
{
    CINFO << "[" << id << "] Indication from: " << addr2id(params.m_srcAddr) << " dst: " << addr2id(params.m_dstAddr) << endl;
    DEFINE_MBUF(mbuf);

    if (p->GetSize() >= COMMLINE_MAX_BUF) {
        CERROR << "Pkt len" << p->GetSize() << " bigger than\n";
        return;
    }

    fflush(stdout);

    mbuf->len           = p->CopyData(mbuf->buf, COMMLINE_MAX_BUF);
    mbuf->src_id        = addr2id(params.m_srcAddr);
    mbuf->dst_id        = addr2id(params.m_dstAddr);
    mbuf->info.sig.lqi  = params.m_mpduLinkQuality;

    SendPacketToStackline(id, mbuf);
}

static void setShortAddress(ns3::Ptr<ns3::LrWpanNetDevice> dev, uint16_t id)
{
    Mac16Address address;
    uint8_t idBuf[2];

    idBuf[0] = (id >> 8) & 0xff;
    idBuf[1] = (id >> 0) & 0xff;
    address.CopyFrom (idBuf);
    dev->GetMac()->SetShortAddress(address);
};

int LrwpanIface::init()
{
    INFO("[Node %i] Initializing\n", GetId());
    Ptr<LrWpanNetDevice> dev = GetDevice(0)->GetObject<LrWpanNetDevice>();
    if (!dev) {
        ERROR("Could not get device\n");
        return FAILURE;
    }

    dev->GetMac()->SetMacMaxFrameRetries(CFG_INT("macMaxRetry", 3));

    /* Set Callbacks */
    INFO("[Node %i] Setting up callbacks\n", GetId());
    dev->GetMac()->SetMcpsDataConfirmCallback(MakeBoundCallback(&LrwpanIface::txConfirm, GetId(), &params, &tx_retries));
    dev->GetMac()->SetMcpsDataIndicationCallback(MakeBoundCallback(&LrwpanIface::rxIndication, GetId(), dev));

    INFO("[Node %i] Setting short address\n", GetId());
    setShortAddress(dev, (uint16_t) GetId());

    /* if(!macAdd) { */
        /* dev->GetMac()->SetMacHeaderAdd(macAdd); */

        //In case where stackline itself add mac header, the airline needs
        //to be set in promiscuous mode so that all the packets with
        //headers are transmitted as is to the stackline on reception
        //dev->GetMac()->SetPromiscuousMode(1);
    /* } */

    return SUCCESS;
}

int LrwpanIface::init(ns3::Ptr<ns3::SpectrumChannel> channel)
{
    init();

    INFO("[Node %i] Associating channel\n", GetId());
    Ptr<LrWpanNetDevice> dev = GetDevice(0)->GetObject<LrWpanNetDevice>();
    dev->SetChannel(channel);

    return SUCCESS;
}

static Mac16Address id2addr(const uint16_t id)
{
    Mac16Address mac;
    uint8_t idstr[2], *ptr=(uint8_t*)&id;
    idstr[1] = ptr[0];
    idstr[0] = ptr[1];
    mac.CopyFrom(idstr);
    return mac;
};

int LrwpanIface::setParam(cl_param_t param, void* src, size_t len)
{
    switch (param) {
    case CL_IEEE_802_15_4_DEST_ADDRESS:
        // TODO change the architecture to have a list of class 
        // with their own set of parameter
        break;
    case CL_IEEE_802_15_4_EXT_ADDRESS: {
        Ptr<LrWpanNetDevice> dev = GetDevice(0)->GetObject<LrWpanNetDevice>();
        if (!dev) {
            CERROR << "get dev failed for lrwpan\n";
            return FAILURE;
        }
        // TODO HANDLE_CMD seems to always pass the max len (2048) as argument.
        if (len == 8) {
            Mac64Address address((char*) src);
            INFO("[Node %i] Setting Ext Addr:%s\n", GetId(), (char*) src);
            dev->GetMac()->SetExtendedAddress (address);
        }
        return SUCCESS;
    }
    case CL_IEEE_802_15_4_PROMISCUOUS: {
        Ptr<LrWpanNetDevice> dev = GetDevice(0)->GetObject<LrWpanNetDevice>();

        if (!dev) {
            CERROR << "get dev failed for lrwpan\n";
            return FAILURE;
        }
        INFO("[Node %i] Set promis mode for lr-wpan iface\n", GetId());
        /* dev->GetMac()->SetPromiscuousMode(1); */
        return SUCCESS;
    }
    case CL_IEEE_802_15_4_TX_POWER: {
        Ptr<LrWpanNetDevice> dev = GetDevice(0)->GetObject<LrWpanNetDevice>();
        LrWpanSpectrumValueHelper svh;
        double txpow = *((double*) src);
        Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txpow, 11);

        if (!dev || !psd) {
            CERROR << "set tx power failed for lrwpan\n";
            return FAILURE;
        }
        INFO("[Node %d] txpower:%f\n", GetId(), txpow);
        dev->GetPhy()->SetTxPowerSpectralDensity(psd);
        return SUCCESS;
    }
    default:
        break;
    }
    return SUCCESS;
}

int LrwpanIface::sendPacket(msg_buf_t *mbuf)
{
    int numNodes = stoi(CFG("numOfNodes"));
    Ptr<LrWpanNetDevice> dev = GetDevice(0)->GetObject<LrWpanNetDevice>();
    Ptr<Packet> p0;

    if (!dev) {
        CERROR << "get dev failed for lrwpan\n";
        return FAILURE;
    }

    if(mbuf->flags & MBUF_IS_CMD) {
        CERROR << "MBUF CMD not handled in Airline... No need!" << endl;
        return FAILURE;
    }

    INFO("[Node %i] Sending %s\n", GetId(), mbuf->buf);

    p0 = Create<Packet> (mbuf->buf, (uint32_t)mbuf->len);
    params.m_srcAddrMode = SHORT_ADDR;
    params.m_dstAddrMode = SHORT_ADDR;
    params.m_dstPanId    = CFG_PANID;
    params.m_dstAddr     = id2addr(mbuf->dst_id);
    params.m_msduHandle  = 0;
    params.m_txOptions   = TX_OPTION_NONE;
    if(mbuf->dst_id != 0xffff) {
        params.m_txOptions = TX_OPTION_ACK;
    }
    tx_retries = 0;

    // If the src node is in promiscuous mode then disable L2-ACK 
    if(IN_RANGE(mbuf->src_id, 0, numNodes)) {
        wf::Nodeinfo *ni=NULL;
        ni = WF_config.get_node_info(mbuf->src_id);
        if(ni && ni->getPromisMode()) {
            params.m_txOptions   = TX_OPTION_NONE;
        }
    }
#if 0
    INFO << "TX DATA: "
         << " src_id=" << GetId()
         << " dst_id=" << params.m_dstAddr
         << " pktlen=" << (int)mbuf->len
         << "\n";
    fflush(stdout);
#endif

    Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev->GetMac(), params, p0);
    return SUCCESS;
}
