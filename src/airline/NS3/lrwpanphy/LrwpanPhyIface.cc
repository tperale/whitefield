#include "lrwpanphy/LrwpanPhyIface.h"

#include "commline/commline.h"
#include "ns3/lr-wpan-spectrum-signal-parameters.h"
#include "ns3/object.h"
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

void LrwpanPhyIface::txConfirm(int id, McpsDataRequestParams* req_params, uint8_t* retries, ns3::McpsDataConfirmParams params)
{
}

void LrwpanPhyIface::rxIndication(int id, ns3::Ptr<ns3::LrWpanNetDevice> dev, ns3::McpsDataIndicationParams params, ns3::Ptr<ns3::Packet> p)
{
}

int LrwpanPhyIface::init()
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
    dev->GetMac()->SetMcpsDataConfirmCallback(MakeBoundCallback(&LrwpanPhyIface::txConfirm, GetId(), &params, &tx_retries));
    dev->GetMac()->SetMcpsDataIndicationCallback(MakeBoundCallback(&LrwpanPhyIface::rxIndication, GetId(), dev));

    INFO("[Node %i] Setting short address\n", GetId());
    /* if(!macAdd) { */
        /* dev->GetMac()->SetMacHeaderAdd(macAdd); */

        //In case where stackline itself add mac header, the airline needs
        //to be set in promiscuous mode so that all the packets with
        //headers are transmitted as is to the stackline on reception
        //dev->GetMac()->SetPromiscuousMode(1);
    /* } */

    return SUCCESS;
}

int LrwpanPhyIface::init(ns3::Ptr<ns3::SpectrumChannel> channel)
{
    init();

    INFO("[Node %i] Associating channel\n", GetId());
    Ptr<LrWpanNetDevice> dev = GetDevice(0)->GetObject<LrWpanNetDevice>();
    dev->SetChannel(channel);

    return SUCCESS;
}

int LrwpanPhyIface::setParam(cl_param_t param, void* src, size_t len)
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

int LrwpanPhyIface::sendPacket(msg_buf_t *mbuf)
{
    Ptr<LrWpanNetDevice> dev = GetDevice(0)->GetObject<LrWpanNetDevice>();
    Ptr<LrWpanPhy> phy = dev->GetPhy();
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

    p0 = Create<Packet> (mbuf->buf, (uint32_t) mbuf->len);

    Simulator::ScheduleNow(&LrWpanPhy::PdDataRequest, phy, (uint32_t) mbuf->len, p0);
    return SUCCESS;
}


void LrwpanPhyIface::startRx()
{
    /* Ptr<LrWpanSpectrumSignalParameters> spectrum =  CreateObject<LrWpanSpectrumSignalParameters>(); */
    /* spectrum->duration = Time(); */
    /* spectrum->psd = (void*) 0; */
    /* spectrum->packetBurst = (void*) 0; */
    /* Simulator::ScheduleNow(&LrWpanPhy::StartRx, phy, spectrum); */
}

void LrwpanPhyIface::stopRx()
{
}

LrwpanPhyIface::LrwpanPhyIface() : IFace() {
};


