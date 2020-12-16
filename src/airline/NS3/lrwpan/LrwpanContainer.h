#ifndef LRPWANCONTAINER_H_
#define LRPWANCONTAINER_H_

#include "IFaceContainer.h"
#include "lrwpan/LrwpanIface.h"
#include <ns3/single-model-spectrum-channel.h>

class LrwpanContainer: public IFaceContainer {
private:
    ns3::Ptr<ns3::SpectrumChannel> channel;
    ns3::Ptr<ns3::PropagationLossModel> plm;

public:
    void Create (uint32_t n)
    {
        INFO("Creating the LrWpan Nodes\n");
        for (uint32_t i = 0; i < n; i++) {
            Ptr<LrwpanIface> iface = ns3::CreateObject<LrwpanIface> ();
            Add(iface);
        }
    }

    int setup();

    LrwpanContainer() : IFaceContainer() {};
};    

#endif /* ifndef LRPWANCONTAINER_H_ */