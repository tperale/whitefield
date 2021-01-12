#ifndef _LRWPANPHYIFACE_H_
#define _LRWPANPHYIFACE_H_

#include "IfaceHandler.h"
#include <bits/stdint-uintn.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/mobility-module.h>

class LrwpanPhyIface : public IFace {
protected:
    static void rxIndication(uint16_t id, Ptr<LrWpanPhy> phy, uint32_t pdsuLength, Ptr<Packet> p, uint8_t lqi);
    static void txConfirm(uint16_t id, Ptr<LrWpanPhy> phy, LrWpanPhyEnumeration status);

    McpsDataRequestParams params;
    uint8_t               tx_retries = 0;

    Ptr<SpectrumChannel>               channel;
    Ptr<ConstantPositionMobilityModel> mobility;
    Ptr<LrWpanPhy>                     m_phy;

public:
    int  setParam(cl_param_t param, void *src, size_t len);
    int  sendPacket(msg_buf_t *mbuf);
    void startRx();
    void stopRx();

    LrwpanPhyIface(Ptr<SpectrumChannel> chan);
};

#endif /* ifndef _LRWPANPHYIFACE_H_ */
