#ifndef _LRWPANPHYIFACE_H_
#define _LRWPANPHYIFACE_H_

#include "IfaceHandler.h"
#include <bits/stdint-uintn.h>
#include <ns3/lr-wpan-module.h>

class LrwpanPhyIface: public IFace {
protected:
    static void rxIndication(int id, ns3::Ptr<ns3::LrWpanNetDevice> dev, ns3::McpsDataIndicationParams params, ns3::Ptr<ns3::Packet> p);
    static void txConfirm(int id, McpsDataRequestParams* req_params, uint8_t* retries, ns3::McpsDataConfirmParams params);
    ns3::Ptr<Node> node;
    McpsDataRequestParams params;
    uint8_t tx_retries = 0;
public:
    int init();
    int init(ns3::Ptr<ns3::SpectrumChannel> channel);
    int setParam(cl_param_t param, void* src, size_t len);
    int sendPacket(msg_buf_t* mbuf);
    void startRx();
    void stopRx();

    LrwpanPhyIface();
};

#endif /* ifndef _LRWPANPHYIFACE_H_ */
