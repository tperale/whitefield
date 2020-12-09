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

#include "commline/commline.h"
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/mobility-module.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/spectrum-value.h>

#include <LrwpanIface.h>
#include <PropagationModel.h>
#include <common.h>
#include <Nodeinfo.h>
#include <Config.h>
#include <IfaceHandler.h>

static Ptr<LrWpanNetDevice> getDev(ifaceCtx_t *ctx, int id)
{
    Ptr<Node> node = ctx->nodes.Get(id); 
    Ptr<LrWpanNetDevice> dev = NULL;

    if (node) {
        dev = node->GetDevice(0)->GetObject<LrWpanNetDevice>();
    }
    return dev;
}

/* static uint8_t wf_ack_status(LrWpanMcpsDataConfirmStatus status) */
/* { */
/*     switch(status) { */
/*         case IEEE_802_15_4_SUCCESS: */
/*             return WF_STATUS_ACK_OK; */
/*         case IEEE_802_15_4_NO_ACK: */
/*             return WF_STATUS_NO_ACK; */
/*         case IEEE_802_15_4_TRANSACTION_OVERFLOW: */
/*         case IEEE_802_15_4_TRANSACTION_EXPIRED: */
/*         case IEEE_802_15_4_CHANNEL_ACCESS_FAILURE: */
/*             return WF_STATUS_ERR;       //can retry later */
/*         case IEEE_802_15_4_INVALID_GTS: */
/*         case IEEE_802_15_4_COUNTER_ERROR: */
/*         case IEEE_802_15_4_FRAME_TOO_LONG: */
/*         case IEEE_802_15_4_UNAVAILABLE_KEY: */
/*         case IEEE_802_15_4_UNSUPPORTED_SECURITY: */
/*         case IEEE_802_15_4_INVALID_PARAMETER: */
/*         default: */
/*             return WF_STATUS_FATAL; */
/*     } */
/* } */

static uint16_t addr2id(const Mac16Address addr)
{
    uint16_t id=0;
    uint8_t str[2], *ptr=(uint8_t *)&id;
    addr.CopyTo(str);
    ptr[1] = str[0];
    ptr[0] = str[1];
    return id;
}

static void DataConfirm (int id, McpsDataConfirmParams params)
{
    /* uint16_t dst_id = addr2id(params.m_addrShortDstAddr); */
    /* uint8_t status; */

    /* if(dst_id == 0xffff) { */
    /*     return; */
    /* } */
#if 0
    INFO << "Sending ACK status" 
         << " src=" << id << " dst=" << dst_id
         << " status=" << params.m_status
         << " retries=" << (int)params.m_retries
         << " pktSize(inc mac-hdr)=" << params.m_pktSz << "\n";
    fflush(stdout);
#endif
    /* status = wf_ack_status(params.m_status); */
    /* SendAckToStackline(id, dst_id, status, params.m_retries+1); */
}

static void DataIndication (int id, McpsDataIndicationParams params,
                            Ptr<Packet> p)
{
    DEFINE_MBUF(mbuf);

    if (p->GetSize() >= COMMLINE_MAX_BUF) {
        CERROR << "Pkt len" << p->GetSize() << " bigger than\n";
        return;
    }

    mbuf->len           = p->CopyData(mbuf->buf, COMMLINE_MAX_BUF);
    mbuf->src_id        = addr2id(params.m_srcAddr);
    mbuf->dst_id        = addr2id(params.m_dstAddr);
    mbuf->info.sig.lqi  = params.m_mpduLinkQuality;
    SendPacketToStackline(id, mbuf);
}

static void setShortAddress(Ptr<LrWpanNetDevice> dev, uint16_t id)
{
    Mac16Address address;
    uint8_t idBuf[2];

    idBuf[0] = (id >> 8) & 0xff;
    idBuf[1] = (id >> 0) & 0xff;
    address.CopyFrom (idBuf);
    dev->GetMac()->SetShortAddress (address);
};

static int setAllNodesParam(NodeContainer & nodes)
{
    Ptr<SingleModelSpectrumChannel> channel;
    string loss_model = CFG("lossModel");
    string del_model = CFG("delayModel");
    /* bool macAdd = CFG_INT("macHeaderAdd", 1); */
    LrWpanSpectrumValueHelper svh;

    if (!loss_model.empty() || !del_model.empty()) {
        channel = CreateObject<SingleModelSpectrumChannel> ();
        if (!channel) {
            return FAILURE;
        }
        if (!loss_model.empty()) {
            static Ptr <PropagationLossModel> plm;
            string loss_model_param = CFG("lossModelParam");
            plm = getLossModel(loss_model, loss_model_param);
            if (!plm) {
                return FAILURE;
            }
            channel->AddPropagationLossModel(plm);
        }
        if (!del_model.empty()) {
            static Ptr <PropagationDelayModel> pdm;
            string del_model_param = CFG("delayModelParam");
            pdm = getDelayModel(del_model, del_model_param);
            if (!pdm) {
                return FAILURE;
            }
            channel->SetPropagationDelayModel(pdm);
        }
    }

    for (NodeContainer::Iterator i = nodes.Begin (); i != nodes.End (); ++i) { 
        Ptr<Node> node = *i; 
        Ptr<LrWpanNetDevice> dev = node->GetDevice(0)->GetObject<LrWpanNetDevice>();
        if (!dev) {
            CERROR << "Coud not get device\n";
            continue;
        }
        dev->GetMac()->SetMacMaxFrameRetries(CFG_INT("macMaxRetry", 3));

        /* Set Callbacks */
        dev->GetMac()->SetMcpsDataConfirmCallback(
                MakeBoundCallback(DataConfirm, node->GetId()));
        dev->GetMac()->SetMcpsDataIndicationCallback(
                MakeBoundCallback(DataIndication, node->GetId()));
        setShortAddress(dev, (uint16_t) node->GetId());

        /* if(!macAdd) { */
            /* dev->GetMac()->SetMacHeaderAdd(macAdd); */

            //In case where stackline itself add mac header, the airline needs
            //to be set in promiscuous mode so that all the packets with
            //headers are transmitted as is to the stackline on reception
            //dev->GetMac()->SetPromiscuousMode(1);
        /* } */
        if (!loss_model.empty() || !del_model.empty()) {
            dev->SetChannel (channel);
        }
    }
    return SUCCESS;
}

int LrwpanIface::setup(ifaceCtx_t *ctx)
{
    INFO("setting up lrwpan\n");
    static LrWpanHelper lrWpanHelper;
    static NetDeviceContainer devContainer = lrWpanHelper.Install(ctx->nodes);
    lrWpanHelper.AssociateToPan (devContainer, CFG_PANID);

    INFO("Using lr-wpan as PHY\n");
    string ns3_capfile = CFG("NS3_captureFile");
    if(!ns3_capfile.empty()) {
        INFO("NS3 Capture File:%s\n", ns3_capfile.c_str());
        lrWpanHelper.EnablePcapAll (ns3_capfile, false /*promiscuous*/);
    }
    setAllNodesParam(ctx->nodes);
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

int LrwpanIface::setParam(ifaceCtx_t *ctx, int id, cl_param_t param, void* src, size_t len)
{
    switch (param) {
    case CL_IEEE_802_15_4_DEST_ADDRESS:
        // TODO change the architecture to have a list of class 
        // with their own set of parameter
        break;
    case CL_IEEE_802_15_4_EXT_ADDRESS: {
        Ptr<LrWpanNetDevice> dev = getDev(ctx, id);
        if (!dev) {
            CERROR << "get dev failed for lrwpan\n";
            return FAILURE;
        }
        // TODO HANDLE_CMD seems to always pass the max len (2048) as argument.
        if (len == 8) {
            Mac64Address address((char*) src);
            INFO("Setting Ext Addr:%s\n", (char*) src);
            dev->GetMac()->SetExtendedAddress (address);
        }
        return SUCCESS;
    }
    case CL_IEEE_802_15_4_PROMISCUOUS: {
        Ptr<LrWpanNetDevice> dev = getDev(ctx, id);

        if (!dev) {
            CERROR << "get dev failed for lrwpan\n";
            return FAILURE;
        }
        INFO("Set promis mode for lr-wpan iface node:%d\n", id);
        /* dev->GetMac()->SetPromiscuousMode(1); */
        return SUCCESS;
    }
    case CL_IEEE_802_15_4_TX_POWER: {
        Ptr<LrWpanNetDevice> dev = getDev(ctx, id);
        LrWpanSpectrumValueHelper svh;
        double txpow = *((double*) src);
        Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txpow, 11);

        if (!dev || !psd) {
            CERROR << "set tx power failed for lrwpan\n";
            return FAILURE;
        }
        INFO("Node:%d txpower:%f\n", id, txpow);
        dev->GetPhy()->SetTxPowerSpectralDensity(psd);
        return SUCCESS;
    }
    default:
        break;
    }
    return SUCCESS;
}

int LrwpanIface::sendPacket(ifaceCtx_t *ctx, int id, msg_buf_t *mbuf)
{
    int numNodes = stoi(CFG("numOfNodes"));
    McpsDataRequestParams params;
    Ptr<LrWpanNetDevice> dev = getDev(ctx, id);
    Ptr<Packet> p0;

    if (!dev) {
        CERROR << "get dev failed for lrwpan\n";
        return FAILURE;
    }

    if(mbuf->flags & MBUF_IS_CMD) {
        CERROR << "MBUF CMD not handled in Airline... No need!" << endl;
        return FAILURE;
    }

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
         << " src_id=" << id
         << " dst_id=" << params.m_dstAddr
         << " pktlen=" << (int)mbuf->len
         << "\n";
    fflush(stdout);
#endif

    Simulator::ScheduleNow(&LrWpanMac::McpsDataRequest, dev->GetMac(), params, p0);
    return SUCCESS;
}


LrwpanIface::LrwpanIface() {
    INFO("LrWPAN Initialized\n");
    fflush(stdout);
}

LrwpanIface::~LrwpanIface() {
}
