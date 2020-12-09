#ifndef _LRWPANIFACE_H_
#define _LRWPANIFACE_H_

#include "IfaceHandler.h"

class LrwpanIface: public IFace {
public:
    int setup(ifaceCtx_t* ctx);
    int setParam(ifaceCtx_t* ctx, int id, cl_param_t param, void* src, size_t len);
    int sendPacket(ifaceCtx_t* ctx, int id, msg_buf_t* mbuf);
    /* int setRxCallback(ifaceCtx_t* ctx); */

    LrwpanIface();
    ~LrwpanIface();
};

#endif /* ifndef _LRWPANIFACE_H_ */
