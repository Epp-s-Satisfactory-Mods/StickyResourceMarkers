#include "SRMRootGameWorldModule.h"

#include "Configuration/ConfigManager.h"
#include "EngineUtils.h"
#include "FGActorRepresentationManager.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGItemDescriptor.h"
#include "FGPlayerController.h"
#include "FGResourceDescriptor.h"
#include "FGResourceNode.h"

#include "SRMClientNodeSubsystem.h"
#include "SRMConfigurationStruct.h"
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
            if (!IsRunningDedicatedServer())
            {
                Local_SubscribeConfigUpdates();
                // Don't do a full config update as the actor representation manager is not set up yet when connecting to a server.
                // When it comes up, it will use the correct values.
                this->UpdateConfigValues();
            }

            break;
        case ELifecyclePhase::POST_INITIALIZATION:
            auto netMode = this->GetWorld()->GetNetMode();
            SRM_LOG("USRMRootGameWorldModule::DispatchLifecycleEvent. netMode: %d", netMode);
            if (netMode < ENetMode::NM_Client)
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

void USRMRootGameWorldModule::Local_SubscribeConfigUpdates()
{
    SRM_LOG("USRMRootGameWorldModule::Local_SubscribeConfigUpdates: START");
    auto gameInstance = this->GetWorld()->GetGameInstance();
    auto configManager = gameInstance->GetSubsystem<UConfigManager>();
    auto modConfigRoot = configManager->GetConfigurationRootSection(ConfigId);

    for (auto& sectionProperty : modConfigRoot->SectionProperties)
    {
        auto& propertyName = sectionProperty.Key;
        auto configProperty = sectionProperty.Value;

        SRM_LOG("USRMRootGameWorldModule::Local_SubscribeConfigUpdates: Examining config property %s", *propertyName);

        if (propertyName.Equals(TEXT("ScanningUnhidesOnCompass")) ||
            propertyName.Equals(TEXT("ScanningUnhidesOnMap")) ||
            propertyName.Equals(TEXT("ResourceCompassViewDistance")) )
        {
            SRM_LOG("USRMRootGameWorldModule::Local_SubscribeConfigUpdates: \tSubscribing UpdateConfig to changes on %s, property at %p!", *propertyName, configProperty);
            configProperty->OnPropertyValueChanged.AddDynamic(this, &USRMRootGameWorldModule::UpdateConfig);
        }
    }

    SRM_LOG("USRMRootGameWorldModule::Local_SubscribeConfigUpdates: END");
}

void USRMRootGameWorldModule::UpdateConfig()
{
    SRM_LOG("USRMRootGameWorldModule::UpdateConfig: START");
    auto config = FSRMConfigurationStruct::GetActiveConfig(this->GetWorld());

    auto oldViewDistance = this->ResourceCompassViewDistance;

    this->UpdateConfigValues();

    if (oldViewDistance != this->ResourceCompassViewDistance)
    {
        SRM_LOG("USRMRootGameWorldModule::UpdateConfig: Old view distance: %s. New view distance: %s",
            *SRMDebugging::GetEnumNameString(oldViewDistance),
            *SRMDebugging::GetEnumNameString(this->ResourceCompassViewDistance));

        this->UpdateHUDRepresentations();
    }

    SRM_LOG("USRMRootGameWorldModule::UpdateConfig: END");
}

void USRMRootGameWorldModule::UpdateConfigValues()
{
    SRM_LOG("USRMRootGameWorldModule::UpdateConfigValues: START");
    auto config = FSRMConfigurationStruct::GetActiveConfig(this->GetWorld());

    this->ScanningUnhidesOnCompass = config.ScanningUnhidesOnCompass;
    this->ScanningUnhidesOnMap = config.ScanningUnhidesOnMap;

    ECompassViewDistance newViewDistance;
    switch (config.ResourceCompassViewDistance)
    {
        // These magic numbers correspond to the blueprint-defined SRMCompassViewDistance enum values, since ModConfig
        // doesn't seem to provide a way to bind C++-defined macros to config values.
    case 0:
        newViewDistance = ECompassViewDistance::CVD_Near;
        break;
    case 1:
        newViewDistance = ECompassViewDistance::CVD_Mid;
        break;
    case 2:
        newViewDistance = ECompassViewDistance::CVD_Far;
        break;
    case 3:
        newViewDistance = ECompassViewDistance::CVD_Always;
        break;
    default:
        SRM_LOG("USRMRootGameWorldModule::UpdateConfigValues: Unknown ResourceCompassViewDistance config value: %d", config.ResourceCompassViewDistance);
        newViewDistance = ECompassViewDistance::CVD_Always;
    }

    this->ResourceCompassViewDistance = newViewDistance;
    SRM_LOG("USRMRootGameWorldModule::UpdateConfigValues: START");
}

void USRMRootGameWorldModule::UpdateHUDRepresentations()
{
    SRM_LOG("USRMRootGameWorldModule::UpdateHUDRepresentations: START");

    // The representations need to be explicitly updated for the HUD to update view ranges
    auto actorRepresentationManager = AFGActorRepresentationManager::Get(this->GetWorld());
    TArray<UFGActorRepresentation*> actorRepresentations;
    actorRepresentationManager->GetAllActorRepresentations(actorRepresentations);
    for (auto rep : actorRepresentations)
    {
        if (rep->IsA(UFGResourceNodeRepresentation::StaticClass()))
        {
            actorRepresentationManager->UpdateRepresentation(rep);
        }
    }

    SRM_LOG("USRMRootGameWorldModule::UpdateHUDRepresentations: END");
}
