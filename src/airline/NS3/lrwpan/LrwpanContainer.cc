#include "lrwpan/LrwpanContainer.h"
#include "lrwpan/LrwpanIface.h"
#include "PropagationModel.h"
#include "common.h"
#include "Config.h"
#include <ns3/lr-wpan-module.h>

void LrwpanContainer::Create (uint32_t n)
{
    INFO("Creating the LrWpan Nodes\n");
    static ns3::LrWpanHelper lrWpanHelper;

    /* bool macAdd = CFG_INT("macHeaderAdd", 1); */
    /* ns3::LrWpanSpectrumValueHelper svh; */
    string loss_model = CFG("lossModel");
    string del_model = CFG("delayModel");

    if (!loss_model.empty() || !del_model.empty()) {
        INFO("Setting up loss_model\n");
        channel = ns3::CreateObject<ns3::SingleModelSpectrumChannel> ();
        if (!channel) {
            ERROR("Failed at channel creation\n");
            return;
        }
        if (!loss_model.empty()) {
            string loss_model_param = CFG("lossModelParam");
            plm = getLossModel(loss_model, loss_model_param);
            if (!plm) {
                ERROR("Failed to get loss model\n");
                return;
            }
            channel->AddPropagationLossModel(plm);
        }
        if (!del_model.empty()) {
            static Ptr <ns3::PropagationDelayModel> pdm;
            string del_model_param = CFG("delayModelParam");
            pdm = getDelayModel(del_model, del_model_param);
            if (!pdm) {
                ERROR("Failed to get delay model\n");
                return;
            }
            channel->SetPropagationDelayModel(pdm);
        }
    } else {
        channel = lrWpanHelper.GetChannel();
    }

    NetDeviceContainer devContainer;
    for (uint32_t i = 0; i < n; i++) {
        INFO("Creating node %i\n", i);
        Ptr<LrwpanIface> iface = ns3::CreateObject<LrwpanIface>(channel);
        Add(iface);
        devContainer.Add(iface->GetDevice(0)->GetObject<LrWpanNetDevice>());
    }

    lrWpanHelper.AssociateToPan(devContainer, CFG_PANID);

    INFO("Using lr-wpan as PHY\n");
    string ns3_capfile = CFG("NS3_captureFile");
    if(!ns3_capfile.empty()) {
        INFO("NS3 Capture File:%s\n", ns3_capfile.c_str());
        lrWpanHelper.EnablePcapAll (ns3_capfile, false /*promiscuous*/);
    }
}

int LrwpanContainer::setup() 
{
    INFO("Initialization of the nodes\n");
    return SUCCESS;
}
