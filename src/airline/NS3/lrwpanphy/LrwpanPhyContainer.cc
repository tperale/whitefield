#include "lrwpanphy/LrwpanPhyContainer.h"
#include "lrwpanphy/LrwpanPhyIface.h"
#include "common.h"
#include "Config.h"
#include "PropagationModel.h"
#include <ns3/lr-wpan-module.h>

void LrwpanPhyContainer::Create(uint32_t n)
{
    INFO("Creating the LrWpan-phy Nodes\n");

    /* LogComponentEnableAll(LOG_PREFIX_FUNC); */
    /* LogComponentEnable("LrWpanPhy", LOG_LEVEL_ALL); */
    /* LogComponentEnable("SingleModelSpectrumChannel", LOG_LEVEL_ALL); */

    static ns3::LrWpanHelper lrWpanHelper;
    channel = lrWpanHelper.GetChannel();
    /* channel = CreateObject<SingleModelSpectrumChannel>(); */

    for (uint32_t i = 0; i < n; i++) {
        Ptr<LrwpanPhyIface> iface = ns3::CreateObject<LrwpanPhyIface>(channel);
        Add(iface);
    }
}
