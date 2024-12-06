#include "RootGameWorldModule_SRM.h"

#include "EngineUtils.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGItemDescriptor.h"
#include "FGResourceDescriptor.h"
#include "FGResourceNode.h"
#include "StickyResourceMarkersRootInstance.h"
#include "SRMLogMacros.h"
#include "SRMDebugging.h"
#include "SRMNodeTrackingSubsystem.h"
#include "SRMResourceRepresentationType.h"


void URootGameWorldModule_SRM::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    SRM_LOG("URootGameWorldModule_SRM::DispatchLifecycleEvent phase: %d. Pointer: %p", phase, this);
    this->IsGameInitializing = true;

    switch (phase)
    {
        case ELifecyclePhase::CONSTRUCTION:
            this->ModSubsystems.Add(ASRMNodeTrackingSubsystem::StaticClass());
            break;
        case ELifecyclePhase::INITIALIZATION:
            this->NodeTrackingSubsystem = ASRMNodeTrackingSubsystem::Get(this->GetWorld());
            UStickyResourceMarkersRootInstance::SetGameWorldModule(this);
            break;
        case ELifecyclePhase::POST_INITIALIZATION:
            RepresentLateResourceNodes();
            RestoreResourceMarkers();
            break;
    }

    Super::DispatchLifecycleEvent(phase);

    this->IsGameInitializing = phase != ELifecyclePhase::POST_INITIALIZATION;
}

void URootGameWorldModule_SRM::RepresentLateResourceNodes()
{
    for (auto node : this->LateInitializedResourceNodes)
    {
        this->AddingFromOtherThanResourceScanner = true;
        node->UpdateNodeRepresentation();
        this->AddingFromOtherThanResourceScanner = false;
    }

    // Totally done with this for this insance of the game world - free up the memory
    this->LateInitializedResourceNodes.Empty();
}

void URootGameWorldModule_SRM::RestoreResourceMarkers()
{
    SRM_LOG("RestoreResourceMarkers: START")
    auto world = this->GetWorld();

    this->AddingFromOtherThanResourceScanner = true;
    for (TActorIterator<AFGResourceNodeBase> It(world); It; ++It)
    {
        auto node = *It;
        if (this->NodeTrackingSubsystem->NodeNeedsRepresentationRestored(node))
        {
            //SRMDebugging::DumpRepresentation("NodeNeedsRepresentationRestored BEFORE UPDATE", node->mResourceNodeRepresentation, false);
            node->ScanResourceNodeScan_Server();
            //SRMDebugging::DumpRepresentation("NodeNeedsRepresentationRestored AFTER UPDATE", node->mResourceNodeRepresentation, false);
        }
    }
    this->AddingFromOtherThanResourceScanner = false;
    SRM_LOG("RestoreResourceMarkers: END")
}
