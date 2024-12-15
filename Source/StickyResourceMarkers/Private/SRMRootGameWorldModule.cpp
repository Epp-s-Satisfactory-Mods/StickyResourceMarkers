#include "SRMRootGameWorldModule.h"

#include "EngineUtils.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGItemDescriptor.h"
#include "FGPlayerController.h"
#include "FGResourceDescriptor.h"
#include "FGResourceNode.h"

#include "SRMClientNodeSubsystem.h"
#include "SRMDebugging.h"
#include "SRMLogMacros.h"
#include "SRMRequestRepresentNodeRCO.h"
#include "SRMRootInstanceModule.h"
#include "SRMServerNodeSubsystem.h"

void USRMRootGameWorldModule::Local_CreateRepresentation_Server(AFGResourceNodeBase* node)
{
    if (!this->NodeRequestRCO)
    {
        // We can't initialize this in DispatchLifecycleEvent because it is not available at that point when connecting to a remote server
        auto firstPlayerController = Cast<AFGPlayerController>(this->GetWorld()->GetFirstPlayerController());
        this->NodeRequestRCO = firstPlayerController->GetRemoteCallObjectOfClass<USRMRequestRepresentNodeRCO>();
    }

    this->NodeRequestRCO->CreateRepresentation_Server(node);
}

bool USRMRootGameWorldModule::IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const
{
    if (node->HasAuthority())
    {
        return this->ServerNodeSubsystem->IsNodeCurrentlyRepresented(node);
    }

    return this->ClientNodeSubsystem->IsNodeCurrentlyRepresented(node);
}

void USRMRootGameWorldModule::SetNodeRepresented(AFGResourceNodeBase* node)
{
    if (node->HasAuthority())
    {
        this->ServerNodeSubsystem->SetNodeRepresented(node);
    }
    else
    {
        this->ClientNodeSubsystem->SetNodeRepresented(node);
    }
}

void USRMRootGameWorldModule::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    SRM_LOG("USRMRootGameWorldModule::DispatchLifecycleEvent phase: %d. Pointer: %p", phase, this);
    this->IsGameInitializing = true;

    switch (phase)
    {
        case ELifecyclePhase::CONSTRUCTION:
            this->ModSubsystems.Add(ASRMClientNodeSubsystem::StaticClass());
            this->ModSubsystems.Add(ASRMServerNodeSubsystem::StaticClass());
            break;
        case ELifecyclePhase::INITIALIZATION:
            this->ClientNodeSubsystem = ASRMClientNodeSubsystem::Get(this->GetWorld());
            SRM_LOG("USRMRootGameWorldModule::DispatchLifecycleEvent. ClientNodeSubsystem: %p", this->ClientNodeSubsystem);
            this->ServerNodeSubsystem = ASRMServerNodeSubsystem::Get(this->GetWorld());
            SRM_LOG("USRMRootGameWorldModule::DispatchLifecycleEvent. ServerNodeSubsystem: %p", this->ServerNodeSubsystem);
            USRMRootInstanceModule::SetGameWorldModule(this);
            break;
        case ELifecyclePhase::POST_INITIALIZATION:
            auto netMode = this->GetWorld()->GetNetMode();
            SRM_LOG("USRMRootGameWorldModule::DispatchLifecycleEvent. netMode: %d", netMode);
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

void USRMRootGameWorldModule::Server_InitializeLateResourceNodes()
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

void USRMRootGameWorldModule::Server_RestoreResourceMarkers()
{
    auto numNodesToRestore = this->ServerNodeSubsystem->NumNodesNeedingRestoration();
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
        if (this->ServerNodeSubsystem->NodeNeedsRepresentationRestored(node))
        {
            SRMDebugging::DumpRepresentation("NodeNeedsRepresentationRestored BEFORE UPDATE", node->mResourceNodeRepresentation, false);
            node->ScanResourceNodeScan_Server();
            SRMDebugging::DumpRepresentation("NodeNeedsRepresentationRestored AFTER UPDATE", node->mResourceNodeRepresentation, false);
        }

        // We can stop looping if we've finishing restoring all the nodes we need
        if (this->ServerNodeSubsystem->NumNodesNeedingRestoration() == 0)
        {
            break;
        }
    }

    SRM_LOG("Server_RestoreResourceMarkers: END")
}
