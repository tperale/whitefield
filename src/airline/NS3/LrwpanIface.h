#ifndef _LRWPANIFACE_H_
#define _LRWPANIFACE_H_

#include "IfaceHandler.h"
#include <bits/stdint-uintn.h>
#include <ns3/lr-wpan-module.h>

class LrwpanIface: public IFace {
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

    LrwpanIface() : IFace() {};
};

#endif /* ifndef _LRWPANIFACE_H_ */
