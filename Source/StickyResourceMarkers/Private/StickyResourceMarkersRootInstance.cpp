#include "StickyResourceMarkersRootInstance.h"

#include "FGActorRepresentation.h"
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

#include "SRMDebugging.h"
#include "SRMHookMacros.h"
#include "SRMLogMacros.h"

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

bool UStickyResourceMarkersRootInstance::TryGetResourceRepresentationType(const AFGResourceNodeBase* resourceNode, EResourceRepresentationType& resourceRepresentationType)
{
    if (!resourceNode)
    {
        return false;
    }

    auto resourceDescriptor = resourceNode->GetResourceClass();
    auto resourceDescriptorName = resourceDescriptor->GetName();

    auto representationType = ResourceRepresentationTypeByDescriptorName.Find(resourceDescriptorName);

    if (representationType)
    {
        resourceRepresentationType = *representationType;
        return true;
    }

    return false;
}

bool UStickyResourceMarkersRootInstance::TryGetResourceRepresentationType(const UFGResourceNodeRepresentation* nodeRep, EResourceRepresentationType& resourceRepresentationType)
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

    // Map the resource descriptors to their (new) ResourceRepresentationTypes
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_Coal_C"), EResourceRepresentationType::RRT_Coal);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_Geyser_C"), EResourceRepresentationType::RRT_Geyser);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_LiquidOil_C"), EResourceRepresentationType::RRT_LiquidOil);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_NitrogenGas_C"), EResourceRepresentationType::RRT_NitrogenGas);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_OreBauxite_C"), EResourceRepresentationType::RRT_OreBauxite);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_OreCopper_C"), EResourceRepresentationType::RRT_OreCopper);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_OreGold_C"), EResourceRepresentationType::RRT_OreGold);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_OreIron_C"), EResourceRepresentationType::RRT_OreIron);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_OreUranium_C"), EResourceRepresentationType::RRT_OreUranium);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_RawQuartz_C"), EResourceRepresentationType::RRT_RawQuartz);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_SAM_C"), EResourceRepresentationType::RRT_SAM);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_Stone_C"), EResourceRepresentationType::RRT_Stone);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_Sulfur_C"), EResourceRepresentationType::RRT_Sulfur);
    ResourceRepresentationTypeByDescriptorName.Add(TEXT("Desc_Water_C"), EResourceRepresentationType::RRT_Water);

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
            gameWorldModule->AddingFromResourceScanner = false;
            scope(self);
            gameWorldModule->AddingFromResourceScanner = true;
            SRM_LOG("AFGBuildableRadarTower::ScanForResources: END %s", *self->GetName());
        });

    // By default, UFGResourceNodeRepresentation::IsOccupied returns IsAllSatellitesOccupied if the resource node is a fracking core.
    // And when a save is being loaded, the radar tower scan runs before all the satellites get added to the fracking core and it
    // seems that, since it has 0 nodes, it returns that all are occupied.  But... why would it return occupied only if ALL Satellites
    // are occupied?  Logically, it's occupied if the main node is occupied (regardless of the satellite nodes) and, visually, the
    // ring around the node tells you how full it is.  Since we don't have a clear way to fix the initialization order and such an
    // obscure bug is not likely to be worked by CSS, we change the logic to avoid the issue and hopefully be more intuitive, anyway.
    SUBSCRIBE_METHOD(UFGResourceNodeRepresentation::IsOccupied,
        [&](auto& scope, const UFGResourceNodeRepresentation* self)
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

    SUBSCRIBE_METHOD(AFGResourceNodeBase::UpdateNodeRepresentation,
        [&](auto& scope, AFGResourceNodeBase* self)
        {
            SRM_LOG("AFGResourceNodeBase::UpdateNodeRepresentation: START %s", *self->GetName());

            auto gameWorldModule = this->GetGameWorldModule();
            if (!gameWorldModule->AddingFromResourceScanner)
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
            SRM_LOG("AFGResourceNodeBase::UpdateNodeRepresentation: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD( UFGResourceNodeRepresentation::SetupResourceNodeRepresentation,
        [&](auto& scope, UFGResourceNodeRepresentation* self, class AFGResourceNodeBase* resourceNode)
        {
            SRM_LOG("UFGResourceNodeRepresentation::SetupResourceNodeRepresentation: START");
            EResourceRepresentationType resourceRepresentationType;
            if (TryGetResourceRepresentationType(resourceNode, resourceRepresentationType))
            {
                this->ResourceTypeNameByResourceRepresentationType.FindOrAdd(resourceRepresentationType, resourceNode->GetResourceName());
            }

            scope(self, resourceNode);

            auto gameWorldModule = this->GetGameWorldModule();
            if (!gameWorldModule->AddingFromResourceScanner && resourceRepresentationType > EResourceRepresentationType::RRT_Default)
            {
                SRM_LOG("UFGResourceNodeRepresentation::SetupResourceNodeRepresentation Scanning from radar tower");

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
                EResourceRepresentationType resourceType;
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
            self->mRepresentationLifeSpan = 0.0;
            scope(self);
            SRM_LOG("AFGResourceScanner::BeginPlay: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGResourceScanner::CreateResourceNodeRepresentations,
        [](auto& scope, AFGResourceScanner* self, const FNodeClusterData& cluster)
        {
            SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: START %d nodes", cluster.Nodes.Num());

            if (cluster.Nodes.Num() > 1)
            {
                for (auto node : cluster.Nodes)
                {
                    self->CreateResourceNodeRepresentations(FNodeClusterData(node));
                }
                scope.Cancel();
                SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: END (CANCELED)");
                return;
            }

            scope(self, cluster);

            SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: END");
        });

    SUBSCRIBE_METHOD(AFGPlayerState::SetMapFilter,
        [](auto& scope, AFGPlayerState* self, ERepresentationType representationType, bool visible)
        {
            SRM_LOG("AFGPlayerState::SetMapFilter: START: representationType: %d, visible: %d", representationType, visible);
            if (representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            {
                if (visible)
                {
                    self->mFilteredOutMapTypes.Add(representationType);
                }
                else
                {
                    self->mFilteredOutMapTypes.Remove(representationType);
                }
                scope.Cancel();
                SRM_LOG("AFGPlayerState::SetMapFilter: END (CANCELED)");
                return;
            }

            scope(self, representationType, visible);
            SRM_LOG("AFGPlayerState::SetMapFilter: END");
        });

    SUBSCRIBE_METHOD(AFGPlayerState::Server_SetMapFilter,
        [](auto& scope, AFGPlayerState* self, ERepresentationType representationType, bool visible)
        {
            SRM_LOG("AFGPlayerState::Server_SetMapFilter: START: representationType: %d, visible: %d", representationType, visible);
            if (representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            {
                if (visible)
                {
                    self->mFilteredOutMapTypes.Add(representationType);
                }
                else
                {
                    self->mFilteredOutMapTypes.Remove(representationType);
                }
                scope.Cancel();
                SRM_LOG("AFGPlayerState::Server_SetMapFilter: END (CANCELED)");
                return;
            }

            scope(self, representationType, visible);
            SRM_LOG("AFGPlayerState::Server_SetMapFilter: END");
        });

    SUBSCRIBE_METHOD(AFGPlayerState::SetCompassFilter,
        [](auto& scope, AFGPlayerState* self, ERepresentationType representationType, bool visible)
        {
            SRM_LOG("AFGPlayerState::SetCompassFilter: START: representationType: %d, visible: %d", representationType, visible);
            if (representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            {
                if (visible)
                {
                    self->mFilteredOutCompassTypes.Add(representationType);
                }
                else
                {
                    self->mFilteredOutCompassTypes.Remove(representationType);
                }
                scope.Cancel();
                SRM_LOG("AFGPlayerState::SetCompassFilter: END (CANCELED)");
                return;
            }

            scope(self, representationType, visible);
            SRM_LOG("AFGPlayerState::SetCompassFilter: END");
        });

    SUBSCRIBE_METHOD(AFGPlayerState::Server_SetCompassFilter,
        [](auto& scope, AFGPlayerState* self, ERepresentationType representationType, bool visible)
        {
            SRM_LOG("AFGPlayerState::Server_SetCompassFilter: START: representationType: %d, visible: %d", representationType, visible);
            if (representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            {
                if (visible)
                {
                    self->mFilteredOutCompassTypes.Add(representationType);
                }
                else
                {
                    self->mFilteredOutCompassTypes.Remove(representationType);
                }
                scope.Cancel();
                SRM_LOG("AFGPlayerState::Server_SetCompassFilter: END (CANCELED)");
                return;
            }

            scope(self, representationType, visible);
            SRM_LOG("AFGPlayerState::Server_SetCompassFilter: END");
        });

    SUBSCRIBE_METHOD(AFGPlayerState::SetMapCategoryCollapsed,
        [](auto& scope, AFGPlayerState* self, ERepresentationType representationType, bool collapsed)
        {
            SRM_LOG("AFGPlayerState::SetMapCategoryCollapsed: START: representationType: %d, visible: %d", representationType, collapsed);
            if (representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            {
                if (collapsed)
                {
                    self->mCollapsedMapCategories.Add(representationType);
                }
                else
                {
                    self->mCollapsedMapCategories.Remove(representationType);
                }
                scope.Cancel();
                SRM_LOG("AFGPlayerState::SetMapCategoryCollapsed: END (CANCELED)");
                return;
            }

            scope(self, representationType, collapsed);
            SRM_LOG("AFGPlayerState::SetMapCategoryCollapsed: END");
        });

    SUBSCRIBE_METHOD(AFGPlayerState::Server_SetMapCategoryCollapsed,
        [](auto& scope, AFGPlayerState* self, ERepresentationType representationType, bool collapsed)
        {
            SRM_LOG("AFGPlayerState::Server_SetMapCategoryCollapsed: START: representationType: %d, visible: %d", representationType, collapsed);
            if (representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            {
                if (collapsed)
                {
                    self->mCollapsedMapCategories.Add(representationType);
                }
                else
                {
                    self->mCollapsedMapCategories.Remove(representationType);
                }

                scope.Cancel();
                SRM_LOG("AFGPlayerState::Server_SetMapCategoryCollapsed: END (CANCELED)");
                return;
            }

            scope(self, representationType, collapsed);
            SRM_LOG("AFGPlayerState::Server_SetMapCategoryCollapsed: END");
        });

    SUBSCRIBE_METHOD(AFGHUD::OnActorRepresentationAdded,
        [](auto& scope, AFGHUD* self, UFGActorRepresentation* actorRepresentation)
        {
            SRM_LOG("AFGHUD::OnActorRepresentationAdded: START %s", *self->GetName());

            //if (actorRepresentation->GetShouldShowInCompass())
            //{
            //    if (auto nodeRep = Cast<UFGResourceNodeRepresentation>(actorRepresentation))
            //    {
            //        ERepresentationType representationType = nodeRep->GetRepresentationType();

            //        if (representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            //        {
            //            TArray<FCompassEntry> copiedEntries(self->GetCompassEntries());
            //            for (auto& entry : copiedEntries)
            //            {
            //                if (auto existingNodeRep = Cast<UFGResourceNodeRepresentation>(entry.RepresentingActor))
            //                {
            //                    if (existingNodeRep->GetResourceNode() == nodeRep->GetResourceNode())
            //                    {
            //                        existingNodeRep->RemoveActorRepresentation();
            //                        break;
            //                    }
            //                }
            //            }
            //        }
            //    }
            //}

            scope(self, actorRepresentation);

            SRMDebugging::DumpCompassEntries("AFGHUD::OnActorRepresentationAdded", self->GetCompassEntries(), true);

            SRM_LOG("AFGHUD::OnActorRepresentationAdded: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGHUD::OnActorRepresentationFiltered,
        [&](auto& scope, AFGHUD* self, ERepresentationType type, bool visible)
        {
            SRM_LOG("AFGHUD::OnActorRepresentationFiltered: START %s, %d, %d", *self->GetName(), type, visible);

            //if (type > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            //{
            //    for (auto& entry : self->GetCompassEntries())
            //    {
            //        if (auto nodeRep = Cast<UFGResourceNodeRepresentation>(entry.RepresentingActor))
            //        {
            //            EResourceRepresentationType resourceRepresentationType;
            //            if (TryGetResourceRepresentationType(nodeRep, resourceRepresentationType) && (ERepresentationType)resourceRepresentationType == type)
            //            {
            //                entry.bEnabled = visible;
            //            }
            //        }
            //    }
            //}
            //else
            //{
            //}
            scope(self, type, visible);

            SRM_LOG("AFGHUD::OnActorRepresentationFiltered: END %s, %d, %d", *self->GetName(), type, visible);
        });

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS

    BEGIN_HOOK(Widget_MapCompass_IconClass, UpdateActor, 108)
        ERepresentationType* representationType = helper.GetLocalVarEnumPtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (*representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(Widget_MapObjectClass, mShowActorDetails, 59)
        ERepresentationType* representationType = helper.GetLocalVarEnumPtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (*representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK_START(Widget_MapClass, GetZOrderForType)
        ERepresentationType* representationType = helper.GetLocalVarEnumPtr<ERepresentationType>(TEXT("representationType"));
        if (*representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapMenuClass, ShouldAddToMenu, 64)
        ERepresentationType* representationType = helper.GetLocalVarEnumPtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (*representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapMenuClass, AddActorRepresentationToMenu, 1447)
        EResourceRepresentationType resourceRepresentationType = *helper.GetLocalVarEnumPtr<EResourceRepresentationType>(TEXT("LocalType"));
        if( resourceRepresentationType > EResourceRepresentationType::RRT_Default )
        {
            helper.SetLocalVarBool(TEXT("K2Node_SwitchEnum_CmpSuccess"), false);
        }
    FINISH_HOOK

    BEGIN_HOOK_RETURN(BPW_MapFilterCategoriesClass, GetCategoryName)
        EResourceRepresentationType resourceRepresentationType = *helper.GetContextVarEnumPtr<EResourceRepresentationType>(TEXT("mRepresentationType"));
        SRM_LOG("BPW_MapFilterCategories::GetCategoryName: resourceRepresentationType %d", resourceRepresentationType);

        if (resourceRepresentationType > EResourceRepresentationType::RRT_Default)
        {
            FText* nameText = this->ResourceTypeNameByResourceRepresentationType.Find(resourceRepresentationType);
            if (!nameText)
            {
                return;
            }

            FText* ReturnValuePointer = helper.GetOutVariablePtr<FTextProperty>(TEXT("ReturnValue"));
            *ReturnValuePointer = *nameText;
        }
    FINISH_HOOK

    BEGIN_HOOK_RETURN(BPW_MapFilterCategoriesClass, CanBeSeenOnCompass)
        ERepresentationType representationType = *helper.GetLocalVarEnumPtr<ERepresentationType>( TEXT("Index"));
        if(representationType == ERepresentationType::RT_Resource || representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default )
        {
            helper.SetOutVarBool(TEXT("ReturnValue"), true);
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapFilterButtonClass, SetActorRepresentation, 378)
        ERepresentationType* representationType = helper.GetLocalVarEnumPtr<ERepresentationType>(TEXT("Temp_byte_Variable"));
        if (*representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
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
