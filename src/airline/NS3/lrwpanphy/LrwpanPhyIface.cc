#include "lrwpanphy/LrwpanPhyIface.h"

#include "commline/commline.h"
#include "ns3/lr-wpan-phy.h"
#include "ns3/lr-wpan-spectrum-signal-parameters.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include <cstdint>
#include <cstdio>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/mobility-module.h>
#include <ns3/spectrum-value.h>

#include <PropagationModel.h>
#include <common.h>
#include <Nodeinfo.h>
#include <Config.h>
#include <IfaceHandler.h>
#include <stdint.h>

void GetSetTRXStateConfirm(uint16_t id, LrWpanPhyEnumeration status)
{
    if (status == 0x09) {
        INFO("[Node %i] Received Set TRX: TX ON\n", id);
    } else if (status == 0x06) {
        INFO("[Node %i] Received Set TRX: RX ON\n", id);
    } else {
        INFO("[Node %i] Received Set TRX: %i\n", id, status);
    }
}

void LrwpanPhyIface::txConfirm(uint16_t id, Ptr<LrWpanPhy> phy, LrWpanPhyEnumeration status)
{
    INFO("[Node %i] TX Confirm %i\n", id, status);
    // TODO Send acknowledgement to packet
    DEFINE_MBUF(mbuf);

    mbuf->src_id          = id;
    mbuf->info.ack.status = status;
    mbuf->flags           = MBUF_IS_ACK;
    mbuf->len             = 0;
    wf::Macstats::set_stats(AL_RX, mbuf);
    cl_sendto_q(MTYPE(STACKLINE, id), mbuf, sizeof(msg_buf_t));
}

void LrwpanPhyIface::rxIndication(uint16_t id, Ptr<LrWpanPhy> phy, uint32_t pdsuLength, Ptr<Packet> p, uint8_t lqi)
{
    INFO("[Node %i] RX indication pdsuLength: %i, lqi:%i \n", id, pdsuLength, lqi);
    DEFINE_MBUF(mbuf);

    if (p->GetSize() >= COMMLINE_MAX_BUF) {
        CERROR << "Pkt len" << p->GetSize() << " bigger than\n";
        return;
    }

    p->CopyData(mbuf->buf, pdsuLength);
    /* mbuf->src_id       = id; */
    mbuf->dst_id       = id;
    mbuf->len          = (uint16_t)pdsuLength;
    mbuf->flags        = MBUF_IS_RX;
    mbuf->info.sig.lqi = lqi;

    wf::Macstats::set_stats(AL_RX, mbuf);
    cl_sendto_q(MTYPE(STACKLINE, id), mbuf, sizeof(msg_buf_t) + mbuf->len);
}

int LrwpanPhyIface::setParam(cl_param_t param, void *src, size_t len)
{
    switch (param) {
        default:
            break;
    }
    return SUCCESS;
}

void SendPacket(Ptr<LrWpanPhy> dev, Ptr<Packet> packet)
{
    dev->PdDataRequest(packet->GetSize(), packet);
}

int LrwpanPhyIface::sendPacket(msg_buf_t *mbuf)
{
    Ptr<Packet> p0;

    INFO("[Node %i] Sending %i bytes\n", GetId(), mbuf->len);
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TRX_OFF);
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_TX_ON);
    p0 = Create<Packet>(mbuf->buf, (uint32_t)mbuf->len);
    Simulator::ScheduleNow(&SendPacket, m_phy, p0);
    return SUCCESS;
}

void LrwpanPhyIface::startRx()
{
    INFO("[Node %i] Set in RX mode\n", GetId());
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_RX_ON);
}

void LrwpanPhyIface::stopRx()
{
    m_phy->PlmeSetTRXStateRequest(IEEE_802_15_4_PHY_IDLE);
}

LrwpanPhyIface::LrwpanPhyIface(Ptr<SpectrumChannel> chan)
    : IFace()
    , channel{ chan }
{
    INFO("Init Node %i\n", GetId());
    m_phy = CreateObject<LrWpanPhy>();
    m_phy->SetChannel(chan);
    chan->AddRx(m_phy);
    mobility = CreateObject<ConstantPositionMobilityModel>();
    m_phy->SetMobility(mobility);

    /* Ptr<MobilityModel> mobility = GetObject<MobilityModel> (); */
    /* if (!mobility) */
    /*     { */
    /*     NS_LOG_WARN ("LrWpanNetDevice: no Mobility found on the node, probably it's not a good idea."); */
    /*     } */
    /* m_phy->SetMobility (mobility); */
    /* Ptr<LrWpanErrorModel> model = CreateObject<LrWpanErrorModel> (); */
    /* m_phy->SetErrorModel (model); */
    /* m_phy->SetDevice (this); */

    m_phy->SetPlmeSetTRXStateConfirmCallback(MakeBoundCallback(&GetSetTRXStateConfirm, (uint16_t)GetId()));
    m_phy->SetPdDataConfirmCallback(MakeBoundCallback(&LrwpanPhyIface::txConfirm, (uint16_t)GetId(), m_phy));
    m_phy->SetPdDataIndicationCallback(MakeBoundCallback(&LrwpanPhyIface::rxIndication, (uint16_t)GetId(), m_phy));
};
