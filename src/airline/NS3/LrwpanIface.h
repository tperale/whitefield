#ifndef _LRWPANIFACE_H_
#define _LRWPANIFACE_H_

#include "IfaceHandler.h"
#include <ns3/lr-wpan-module.h>

class LrwpanIface: public IFace {
protected:
    static void rxIndication(int id, Ptr<LrWpanNetDevice> dev, McpsDataIndicationParams params, Ptr<Packet> p);
    static void rxConfirm(int id, Ptr<LrWpanNetDevice> dev, McpsDataConfirmParams params);
public:
    int setup(ifaceCtx_t* ctx);
    int setParam(ifaceCtx_t* ctx, int id, cl_param_t param, void* src, size_t len);
    int sendPacket(ifaceCtx_t* ctx, int id, msg_buf_t* mbuf);

     LrwpanIface();
    ~LrwpanIface();
};

#endif /* ifndef _LRWPANIFACE_H_ */
