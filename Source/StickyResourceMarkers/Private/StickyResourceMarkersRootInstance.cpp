#include "StickyResourceMarkersRootInstance.h"

#include "FGActorRepresentation.h"
#include "FGActorRepresentationManager.h"
#include "FGBuildableRadarTower.h"
#include "FGPlayerState.h"
#include "FGResourceNodeBase.h"
#include "FGResourceNodeFrackingCore.h"
#include "FGResourceNodeRepresentation.h"
#include "FGResourceScanner.h"
#include "FGHUD.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "RootGameWorldModule_SRM.h"
#include "SRMNodeTrackingSubsystem.h"

#include "SRMDebugging.h"
#include "SRMHookMacros.h"
#include "SRMLogMacros.h"
#include "SRMResourceRepresentationType.h"

URootGameWorldModule_SRM* UStickyResourceMarkersRootInstance::CurrentGameWorldModule = nullptr;

void UStickyResourceMarkersRootInstance::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    SRM_LOG("UStickyResourceMarkersRootInstance::DispatchLifecycleEvent: Phase %d", phase);

    switch (phase)
    {
    case ELifecyclePhase::CONSTRUCTION:
        Initialize();
        break;
    }

    Super::DispatchLifecycleEvent(phase);
}

bool UStickyResourceMarkersRootInstance::TryGetResourceRepresentationType(const AFGResourceNodeBase* resourceNode, ESRMResourceRepresentationType& resourceRepresentationType)
{
    if (!resourceNode)
    {
        return false;
    }

    auto resourceDescriptor = resourceNode->GetResourceClass();
    auto resourceDescriptorName = resourceDescriptor->GetFName();

    auto representationType = this->ResourceRepresentationTypeByDescriptorName.Find(resourceDescriptorName);

    if (representationType)
    {
        resourceRepresentationType = *representationType;
        return true;
    }

    return false;
}

bool UStickyResourceMarkersRootInstance::TryGetResourceRepresentationType(const UFGResourceNodeRepresentation* nodeRep, ESRMResourceRepresentationType& resourceRepresentationType)
{
    if (nodeRep->IsCluster())
    {
        // We won't end up adding clusters to the compass/map/menu, so just use the default
        return false;
    }

    return TryGetResourceRepresentationType(nodeRep->GetResourceNode(), resourceRepresentationType);
}

void UStickyResourceMarkersRootInstance::Initialize()
{
    SRM_LOG("UStickyResourceMarkersRootInstance::Initialize");

    if (WITH_EDITOR)
    {
        SRM_LOG("UStickyResourceMarkersRootInstance::Initialize: Not initializing anything because WITH_EDITOR is true!");
        return;
    }

    // Map the resource descriptor names to their ResourceRepresentationTypes. We use FNames because they are interned and so
    // lookups shouldn't need to compare the whole string
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_Coal_C"), ESRMResourceRepresentationType::RRT_Coal);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_Geyser_C"), ESRMResourceRepresentationType::RRT_Geyser);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_LiquidOil_C"), ESRMResourceRepresentationType::RRT_LiquidOil);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_NitrogenGas_C"), ESRMResourceRepresentationType::RRT_NitrogenGas);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_OreBauxite_C"), ESRMResourceRepresentationType::RRT_OreBauxite);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_OreCopper_C"), ESRMResourceRepresentationType::RRT_OreCopper);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_OreGold_C"), ESRMResourceRepresentationType::RRT_OreGold);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_OreIron_C"), ESRMResourceRepresentationType::RRT_OreIron);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_OreUranium_C"), ESRMResourceRepresentationType::RRT_OreUranium);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_RawQuartz_C"), ESRMResourceRepresentationType::RRT_RawQuartz);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_SAM_C"), ESRMResourceRepresentationType::RRT_SAM);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_Stone_C"), ESRMResourceRepresentationType::RRT_Stone);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_Sulfur_C"), ESRMResourceRepresentationType::RRT_Sulfur);
    ResourceRepresentationTypeByDescriptorName.Add(FName("Desc_Water_C"), ESRMResourceRepresentationType::RRT_Water);

    SRM_LOG("Registering hooks...");

    RegisterDebugHooks();

    if (SRM_DEBUGGING_ENABLED && !SRM_DEBUGGING_REGISTER_MOD_HOOKS)
    {
        SRM_LOG("UStickyResourceMarkersRootInstance::Initialize: Not registering mod hooks because SRM_DEBUGGING_ENABLED is 1 and SRM_DEBUGGING_REGISTER_MOD_HOOKS is 0!");
        return;
    }

    SUBSCRIBE_METHOD(AFGBuildableRadarTower::ScanForResources,
        [&](auto& scope, AFGBuildableRadarTower* self)
        {
            SRM_LOG("AFGBuildableRadarTower::ScanForResources: START %s", *self->GetName());
            auto gameWorldModule = this->GetGameWorldModule();
            gameWorldModule->AddingFromOtherThanResourceScanner = true;
            scope(self);
            gameWorldModule->AddingFromOtherThanResourceScanner = false;
            SRM_LOG("AFGBuildableRadarTower::ScanForResources: END %s", *self->GetName());
        });

    // By default, UFGResourceNodeRepresentation::IsOccupied returns IsAllSatellitesOccupied if the resource node is a fracking core.
    // And when a save is being loaded, the radar tower scan runs before all the satellites get added to the fracking core and it
    // seems that, since it has 0 nodes, it returns that all are occupied.  But... why would it return occupied only if ALL Satellites
    // are occupied?  Logically, it's occupied if the main node is occupied (regardless of the satellite nodes) and, visually, the
    // ring around the node tells you how full it is.  Since we don't have a clear way to fix the initialization order and such an
    // obscure bug is not likely to be worked by CSS, we change the logic to avoid the issue and hopefully be more intuitive, anyway.
    SUBSCRIBE_METHOD(UFGResourceNodeRepresentation::IsOccupied,
        [](auto& scope, const UFGResourceNodeRepresentation* self)
        {
            if (auto resourceNode = self->GetResourceNode())
            {
                auto isOccupied = resourceNode->IsOccupied();
                scope.Override(isOccupied);
                return isOccupied;
            }

            scope.Override(self->mIsOccupied);
            return self->mIsOccupied;
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::ScanResourceNodeScan_Server,
        [](auto& scope, AFGResourceNodeBase* self)
        {
            SRM_LOG("AFGResourceNodeBase::ScanResourceNodeScan_Server: START");

            SRM_LOG("AFGResourceNodeBase::ScanResourceNodeScan_Server: END");
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::ScanResourceNode_Local,
        [](auto& scope, AFGResourceNodeBase* self, float lifeSpan)
        {
            SRM_LOG("AFGResourceNodeBase::ScanResourceNode_Local: START, %d", lifeSpan);

            SRM_LOG("AFGResourceNodeBase::ScanResourceNode_Local: END");
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::UpdateNodeRepresentation,
        [&](auto& scope, AFGResourceNodeBase* self)
        {
            SRM_LOG("AFGResourceNodeBase::UpdateNodeRepresentation: START %s, FName: %s", *self->GetName(), *self->GetFName().ToString());

            auto gameWorldModule = this->GetGameWorldModule();
            auto nodeSubsystem = gameWorldModule->NodeTrackingSubsystem;

            if (nodeSubsystem->IsNodeCurrentlyRepresented(self))
            {
                SRM_LOG("AFGResourceNodeBase::UpdateNodeRepresentation: END (CANCELED - ALREADY REPRESENTED) %s", *self->GetName());
                scope.Cancel();
                return;
            }

            if (gameWorldModule->IsGameInitializing)
            {
                if (auto frackingCore = Cast<AFGResourceNodeFrackingCore>(self))
                {
                    int32 totalSatellites = 0;
                    frackingCore->GetNumOccupiedSatellites(totalSatellites);

                    // If there are no satellites, the game is still loading. This can happen when loading a game with a radar tower.
                    // Store it to be initialized later by the game world module and cancel this initialization.
                    if (totalSatellites == 0)
                    {
                        gameWorldModule->LateInitializedResourceNodes.Add(frackingCore);
                        SRM_LOG("AFGResourceNodeBase::UpdateNodeRepresentation: END (CANCELED) %s", *self->GetName());
                        scope.Cancel();
                        return;
                    }
                }
            }

            scope(self);

            // Late in testing, I discovered this might not actually be right because UpdateNodeRepresentation doesn't do anything if there
            // is no representation yet.
            nodeSubsystem->SetNodeRepresented(self);

            SRM_LOG("AFGResourceNodeBase::UpdateNodeRepresentation: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD( UFGResourceNodeRepresentation::SetupResourceNodeRepresentation,
        [&](auto& scope, UFGResourceNodeRepresentation* self, class AFGResourceNodeBase* resourceNode)
        {
            SRM_LOG("UFGResourceNodeRepresentation::SetupResourceNodeRepresentation: START");

            // Here, we make sure we have the resource type name cached for this resource type. I tried multiple different methods of
            // constructing this map on game initialization but the available methods of getting all item descriptors all crash when
            // you attempt to access their item names at any point in initialization.
            ESRMResourceRepresentationType resourceRepresentationType;
            if (TryGetResourceRepresentationType(resourceNode, resourceRepresentationType))
            {
                if (!this->ResourceTypeNameByResourceRepresentationType.Contains(resourceRepresentationType))
                {
                    this->ResourceTypeNameByResourceRepresentationType.Add(resourceRepresentationType, resourceNode->GetResourceName());
                }
            }

            scope(self, resourceNode);

            // If this is not being added from the resource scanner, then it's being added with the assumption that it's only going on the map.
            // Because we make all resource markers available to the compass, we need to finish setting it up for the compass.
            auto gameWorldModule = this->GetGameWorldModule();
            if (gameWorldModule->AddingFromOtherThanResourceScanner && resourceRepresentationType > ESRMResourceRepresentationType::RRT_Default)
            {
                self->mRepresentationColor = FLinearColor::White;
                self->mShouldShowInCompass = true;
                self->mCompassViewDistance = ECompassViewDistance::CVD_Always;
            }

            SRM_LOG("UFGResourceNodeRepresentation::SetupResourceNodeRepresentation END");
        });

    SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::GetRepresentationType,
        GetMutableDefault<UFGActorRepresentation>(),
        [&](auto& scope, const UFGActorRepresentation* self)
        {
            if (auto nodeRep = Cast<UFGResourceNodeRepresentation>(self))
            {
                ESRMResourceRepresentationType resourceType;
                if (TryGetResourceRepresentationType(nodeRep, resourceType))
                {
                    scope.Override((ERepresentationType)resourceType);
                    return (ERepresentationType)resourceType;
                }
            }

            auto representationType = scope(self);
            return representationType;
        });

    SUBSCRIBE_METHOD_VIRTUAL(AFGResourceScanner::BeginPlay,
        GetMutableDefault<AFGResourceScanner>(),
        [](auto& scope, AFGResourceScanner* self)
        {
            SRM_LOG("AFGResourceScanner::BeginPlay: START %s", *self->GetName());
            self->mRepresentationLifeSpan = 0.0; // So that resource representations never disappear
            scope(self);
            SRM_LOG("AFGResourceScanner::BeginPlay: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGResourceScanner::CreateResourceNodeRepresentations,
        [&](auto& scope, AFGResourceScanner* self, const FNodeClusterData& cluster)
        {
            SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: START %d nodes", cluster.Nodes.Num());

            // Cluster representations don't have a resource descriptor readily available, so it would take
            // workarounds to support them. Plus, because they don't have their descriptor, the map menu already
            // has a bug where it adds empty lines to the menu on scan.  Overall, clusters icons add so little value
            // (and, to me, negative value) that they aren't worth supporting.
            // This splits clusters into individual nodes so that we don't have to deal with them at all anywhere.
            if (cluster.Nodes.Num() > 1)
            {
                for (auto node : cluster.Nodes)
                {
                    self->CreateResourceNodeRepresentations(FNodeClusterData(node));
                }
                scope.Cancel();
                SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: END (CANCELED - MULTIPLE)");
                return;
            }

            scope(self, cluster);

            SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: END");
        });

    // These blueprint hooks all hook very specific locations in their blueprint functions to trick the map and compass UIs into treating them like
    // they have RepresentationType RT_Resource and/or to add the bits of custom behavior (like putting them in different categories and making them
    // filterable on the compass). If an underlying blueprint function changes even a little, there's a good chance these will have to be updated.
    // Look in the HookSnapshots folder in the root of the repo for a README that explains how to find the differences.

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS

    BEGIN_HOOK(Widget_MapCompass_IconClass, UpdateActor, 108)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (*representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(Widget_MapObjectClass, mShowActorDetails, 59)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (*representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK_START(Widget_MapClass, GetZOrderForType)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("representationType"));
        if (*representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapMenuClass, ShouldAddToMenu, 64)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (*representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapMenuClass, AddActorRepresentationToMenu, 1447)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("LocalType"));
        if (*representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            localHelper->SetBoolVariable(TEXT("K2Node_SwitchEnum_CmpSuccess"), false);
        }
    FINISH_HOOK

    BEGIN_HOOK_RETURN(BPW_MapFilterCategoriesClass, GetCategoryName)
        auto contextHelper = helper.GetContextVariableHelper();
        ERepresentationType* representationType = contextHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("mRepresentationType"));
        ESRMResourceRepresentationType resourceRepresentationType = (ESRMResourceRepresentationType)(*representationType);
        if (*representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            FText* nameText = this->ResourceTypeNameByResourceRepresentationType.Find(resourceRepresentationType);
            if (!nameText)
            {
                return;
            }

            auto outHelper = helper.GetOutVariableHelper();
            FText* returnValuePointer = outHelper->GetVariablePtr<FTextProperty>(TEXT("ReturnValue"));
            *returnValuePointer = *nameText;
        }
    FINISH_HOOK

    BEGIN_HOOK_RETURN(BPW_MapFilterCategoriesClass, CanBeSeenOnCompass)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType representationType = *localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("Index"));
        if(representationType == ERepresentationType::RT_Resource || representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default )
        {
            auto outHelper = helper.GetOutVariableHelper();
            outHelper->SetBoolVariable(TEXT("ReturnValue"), true);
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapFilterButtonClass, SetActorRepresentation, 378)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("Temp_byte_Variable"));
        if (*representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    SRM_LOG("Hooks registered...");
}

void UStickyResourceMarkersRootInstance::RegisterDebugHooks()
{
    if (!SRM_DEBUGGING_ENABLED) return;

    SRMDebugging::RegisterNativeDebugHooks();

    SRMDebugging::RegisterDebugHooks_Widget_MapContainer(Widget_MapContainerClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_MapTab(Widget_MapTabClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_Map(Widget_MapClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_MapObject(Widget_MapObjectClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_MapCompass_Icon(Widget_MapCompass_IconClass.Get());

    SRMDebugging::RegisterDebugHooks_BPW_MapMenu(BPW_MapMenuClass.Get());
    SRMDebugging::RegisterDebugHooks_BPW_MapFilterCategories(BPW_MapFilterCategoriesClass.Get());
    //SRMDebugging::RegisterDebugHooks_BPW_MapFiltersSubCategory(BPW_MapFiltersSubCategoryClass.Get());
    SRMDebugging::RegisterDebugHooks_BPW_MapFilterButton(BPW_MapFilterButtonClass.Get());
}
