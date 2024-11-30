#include "RootGameWorldModule_SRM.h"
#include "StickyResourceMarkersRootInstance.h"
#include "SRMLogMacros.h"

#include "FGResourceNode.h"

void URootGameWorldModule_SRM::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    SRM_LOG("URootGameWorldModule_SRM::DispatchLifecycleEvent phase: %d. Pointer: %p", phase, this);

    switch (phase)
    {
        case ELifecyclePhase::INITIALIZATION:
            UStickyResourceMarkersRootInstance::SetGameWorldModule(this);
            break;
        case ELifecyclePhase::POST_INITIALIZATION:
            RegisterLateResourceNodes();
            break;
    }

    Super::DispatchLifecycleEvent(phase);
}

void URootGameWorldModule_SRM::RegisterLateResourceNodes()
{
    for (auto node : this->LateInitializedResourceNodes)
    {
        this->AddingFromResourceScanner = false;
        node->UpdateNodeRepresentation();
        this->AddingFromResourceScanner = true;
    }
}
