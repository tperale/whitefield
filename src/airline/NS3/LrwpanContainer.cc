#include "LrwpanContainer.h"
#include "LrwpanIface.h"
#include "PropagationModel.h"
#include "common.h"
#include "Config.h"
#include <ns3/lr-wpan-module.h>

int LrwpanContainer::setup() 
{
    INFO("setting up lrwpan\n");
    static ns3::LrWpanHelper lrWpanHelper;
    NetDeviceContainer devContainer;
    for (NodeContainer::Iterator i = this->Begin (); i != this->End (); i++)
    {
        Ptr<Node> node = *i;

        Ptr<LrWpanNetDevice> netDevice = CreateObject<LrWpanNetDevice> ();
        netDevice->SetChannel (lrWpanHelper.GetChannel());
        node->AddDevice(netDevice);
        netDevice->SetNode(node);
        devContainer.Add(netDevice);
    }


    lrWpanHelper.AssociateToPan(devContainer, CFG_PANID);

    INFO("Using lr-wpan as PHY\n");
    string ns3_capfile = CFG("NS3_captureFile");
    if(!ns3_capfile.empty()) {
        INFO("NS3 Capture File:%s\n", ns3_capfile.c_str());
        lrWpanHelper.EnablePcapAll (ns3_capfile, false /*promiscuous*/);
    }

    /* bool macAdd = CFG_INT("macHeaderAdd", 1); */
    ns3::LrWpanSpectrumValueHelper svh;
    string loss_model = CFG("lossModel");
    string del_model = CFG("delayModel");

    if (!loss_model.empty() || !del_model.empty()) {
        INFO("Setting up loss_model\n");
        channel = ns3::CreateObject<ns3::SingleModelSpectrumChannel> ();
        if (!channel) {
            ERROR("Failed at channel creation\n");
            return FAILURE;
        }
        if (!loss_model.empty()) {
            string loss_model_param = CFG("lossModelParam");
            plm = getLossModel(loss_model, loss_model_param);
            if (!plm) {
                ERROR("Failed to get loss model\n");
                return FAILURE;
            }
            channel->AddPropagationLossModel(plm);
        }
        if (!del_model.empty()) {
            static Ptr <ns3::PropagationDelayModel> pdm;
            string del_model_param = CFG("delayModelParam");
            pdm = getDelayModel(del_model, del_model_param);
            if (!pdm) {
                ERROR("Failed to get delay model\n");
                return FAILURE;
            }
            channel->SetPropagationDelayModel(pdm);
        }
    }

    INFO("Initialization of the nodes\n");
    for (NodeContainer::Iterator i = this->Begin (); i != this->End (); i++)
    {
        Ptr<Node> node = *i;
        INFO("Initialization of node %i\n", node->GetId());
        Ptr<LrwpanIface> iface = node->GetObject<LrwpanIface>();
        if (channel) {
            iface->init(channel);
        } else {
            iface->init();
        }
    }

    return SUCCESS;
}
