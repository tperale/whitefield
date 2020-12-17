#ifndef _LRWPANIFACE_H_
#define _LRWPANIFACE_H_

#include "IfaceHandler.h"
#include <bits/stdint-uintn.h>
#include <ns3/lr-wpan-module.h>

class LrwpanIface: public IFace {
protected:
    static void rxIndication(uint16_t id, ns3::McpsDataIndicationParams params, ns3::Ptr<ns3::Packet> p);
    static void txConfirm(uint16_t id, McpsDataRequestParams* req_params, uint8_t* retries, ns3::McpsDataConfirmParams params);
    Ptr<Node> node;
    Ptr<SpectrumChannel> channel;
    Ptr<LrWpanNetDevice> dev;

    McpsDataRequestParams params;
    uint8_t tx_retries = 0;
public:
    Ptr<LrWpanNetDevice> getDev();
    int setParam(cl_param_t param, void* src, size_t len);
    int sendPacket(msg_buf_t* mbuf);

    LrwpanIface(Ptr<SpectrumChannel> channel);
};

#endif /* ifndef _LRWPANIFACE_H_ */
