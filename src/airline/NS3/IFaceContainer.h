#ifndef IFACECONTAINER_H_
#define IFACECONTAINER_H_ 
#include "IfaceHandler.h"
#include "common.h"
#include "ns3/lr-wpan-net-device.h"
#include <cstdint>
#include <cstdio>
#include <stdint.h>
#include <vector>
#include <ns3/node-container.h>
#include <ns3/node.h>
extern "C" {
#include "commline/commline.h"
}

class IFaceContainer: public ns3::NodeContainer {
public:
    /**
     * Redefinition of the NodeContainer Create that need to be defined by each
     * class inheriting from this one to specifically allocate the "iface".
     */
    virtual void Create (uint32_t n) = 0;
    /**
     * Setup function initializing each nodes and configuring the
     * NodeContainer.
     */
    virtual int setup() = 0;

    int setParam(uint16_t id, cl_param_t param, void* src, size_t len) {
        Ptr<IFace> node = Get(id)->GetObject<IFace>();
        return node->setParam(param, src, len);
    }

    int sendPacket(uint16_t id, msg_buf_t* mbuf) {
        Ptr<IFace> node = Get(id)->GetObject<IFace>();
        return node->sendPacket(mbuf);
    }

    IFaceContainer() : ns3::NodeContainer() {};
};

#endif /* ifndef IFACECONTAINER_H_ */
