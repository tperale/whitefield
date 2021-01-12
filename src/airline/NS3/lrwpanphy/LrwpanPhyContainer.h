#ifndef LRPWANPHYCONTAINER_H_
#define LRPWANPHYCONTAINER_H_

#include "IFaceContainer.h"
#include "lrwpanphy/LrwpanPhyIface.h"
#include <ns3/single-model-spectrum-channel.h>

class LrwpanPhyContainer : public IFaceContainer {
private:
    ns3::Ptr<ns3::SpectrumChannel>      channel;
    ns3::Ptr<ns3::PropagationLossModel> plm;

public:
    void Create(uint32_t n);

    LrwpanPhyContainer()
        : IFaceContainer(){};
};

#endif /* ifndef LRPWANPHYCONTAINER_H_ */
