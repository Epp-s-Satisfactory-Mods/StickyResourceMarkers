#include "RootGameWorldModule_SRM.h"

#include "EngineUtils.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGItemDescriptor.h"
#include "FGPlayerController.h"
#include "FGResourceDescriptor.h"
#include "FGResourceNode.h"
#include "StickyResourceMarkersRootInstance.h"
#include "SRMLogMacros.h"
#include "SRMClientNodeSubsystem.h"
#include "SRMDebugging.h"
#include "SRMNodeTrackingSubsystem.h"
#include "SRMRequestRepresentNodeRCO.h"


void URootGameWorldModule_SRM::Local_CreateRepresentation_Server(AFGResourceNodeBase* node)
{
    if (!this->NodeRequestRCO)
    {
        // We can't initialize this in DispatchLifecycleEvent because it is not available at that point when connecting to a remote server
        auto firstPlayerController = Cast<AFGPlayerController>(this->GetWorld()->GetFirstPlayerController());
        this->NodeRequestRCO = firstPlayerController->GetRemoteCallObjectOfClass<USRMRequestRepresentNodeRCO>();
    }

    this->NodeRequestRCO->CreateRepresentation_Server(node);
}

bool URootGameWorldModule_SRM::IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const
{
    if (node->HasAuthority())
    {
        return this->NodeTrackingSubsystem->IsNodeCurrentlyRepresented(node);
    }

    return this->ClientNodeSubsystem->IsNodeCurrentlyRepresented(node);
}

void URootGameWorldModule_SRM::SetNodeRepresented(AFGResourceNodeBase* node)
{
    if (node->HasAuthority())
    {
        this->NodeTrackingSubsystem->SetNodeRepresented(node);
    }
    else
    {
        this->ClientNodeSubsystem->SetNodeRepresented(node);
    }
}

void URootGameWorldModule_SRM::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    SRM_LOG("URootGameWorldModule_SRM::DispatchLifecycleEvent phase: %d. Pointer: %p", phase, this);
    this->IsGameInitializing = true;

    switch (phase)
    {
        case ELifecyclePhase::CONSTRUCTION:
            this->ModSubsystems.Add(ASRMClientNodeSubsystem::StaticClass());
            this->ModSubsystems.Add(ASRMNodeTrackingSubsystem::StaticClass());
            break;
        case ELifecyclePhase::INITIALIZATION:
            this->ClientNodeSubsystem = ASRMClientNodeSubsystem::Get(this->GetWorld());
            SRM_LOG("URootGameWorldModule_SRM::DispatchLifecycleEvent. ClientNodeSubsystem: %p", this->ClientNodeSubsystem);
            this->NodeTrackingSubsystem = ASRMNodeTrackingSubsystem::Get(this->GetWorld());
            SRM_LOG("URootGameWorldModule_SRM::DispatchLifecycleEvent. NodeTrackingSubsystem: %p", this->NodeTrackingSubsystem);
            UStickyResourceMarkersRootInstance::SetGameWorldModule(this);
            break;
        case ELifecyclePhase::POST_INITIALIZATION:
            auto netMode = this->GetWorld()->GetNetMode();
            SRM_LOG("URootGameWorldModule_SRM::DispatchLifecycleEvent. netMode: %d", netMode);
            if (netMode == ENetMode::NM_DedicatedServer || netMode == ENetMode::NM_ListenServer || netMode == ENetMode::NM_Standalone)
            {
                Server_InitializeLateResourceNodes();
                Server_RestoreResourceMarkers();
            }
            break;
    }

    Super::DispatchLifecycleEvent(phase);

    if (phase == ELifecyclePhase::POST_INITIALIZATION)
    {
        this->IsGameInitializing = false;
    }
}

void URootGameWorldModule_SRM::Server_InitializeLateResourceNodes()
{
    SRM_LOG("Server_InitializeLateResourceNodes: START. There are %d nodes to late initialize", this->LateInitializedResourceNodes.Num())
    for (auto node : this->LateInitializedResourceNodes)
    {
        node->UpdateNodeRepresentation();
    }

    // Totally done with this for this instance of the game world - might as well free up the memory
    this->LateInitializedResourceNodes.Empty();
    SRM_LOG("Server_InitializeLateResourceNodes: END")
}

void URootGameWorldModule_SRM::Server_RestoreResourceMarkers()
{
    auto numNodesToRestore = this->NodeTrackingSubsystem->NumNodesNeedingRestoration();
    SRM_LOG("Server_RestoreResourceMarkers: START. There are %d nodes to restore", numNodesToRestore)
    auto world = this->GetWorld();

    if (numNodesToRestore == 0)
    {
        SRM_LOG("Server_RestoreResourceMarkers: END (No nodes to restore)")
        return;
    }

    for (TActorIterator<AFGResourceNodeBase> It(world); It; ++It)
    {
        auto node = *It;
        if (this->NodeTrackingSubsystem->NodeNeedsRepresentationRestored(node))
        {
            SRMDebugging::DumpRepresentation("NodeNeedsRepresentationRestored BEFORE UPDATE", node->mResourceNodeRepresentation, false);
            node->ScanResourceNodeScan_Server();
            SRMDebugging::DumpRepresentation("NodeNeedsRepresentationRestored AFTER UPDATE", node->mResourceNodeRepresentation, false);
        }

        // We can stop looping if we've finishing restoring all the nodes we need
        if (this->NodeTrackingSubsystem->NumNodesNeedingRestoration() == 0)
        {
            break;
        }
    }

    SRM_LOG("Server_RestoreResourceMarkers: END")
}
