#include "StickyResourceMarkersRootInstance.h"
#include "StickyResourceMarkers.h"

#include "FGActorRepresentation.h"
#include "FGActorRepresentationManager.h"
#include "FGMapWidget.h"
#include "FGMapObjectWidget.h"
#include "FGResourceNodeBase.h"
#include "FGResourceNodeRepresentation.h"
#include "FGResourceScanner.h"
#include "FGHUD.h"
#include "FGInteractWidget.h"
#include "FGUserWidget.h"
#include "Field.h"
#include "UObjectGlobals.h"
#include "FGBuildGunBuild.h"
#include "FGUserWidget.h"
#include "PanelWidget.h"
#include "VerticalBox.h"
#include "GameFramework/Pawn.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"
#include "UnrealType.h"

#include "SRMDebugging.h"
#include "SRMHookMacros.h"
#include "SRMLogMacros.h"

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

bool UStickyResourceMarkersRootInstance::TryGetResourceRepresentationType(const UFGResourceNodeRepresentation* nodeRep, EResourceRepresentationType& resourceRepresentationType)
{
    if (nodeRep->IsCluster() || !nodeRep->GetResourceNode())
    {
        // We won't end up adding clusters or representations without a resource node to the map/menu, so just use the default
        return false;
    }

    auto resourceNode = nodeRep->GetResourceNode();
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

    SUBSCRIBE_METHOD(AFGActorRepresentationManager::AddRepresentation,
        [](auto& scope, AFGActorRepresentationManager* self, UFGActorRepresentation* actorRepresentation)
        {
            SRM_LOG("AFGActorRepresentationManager::AddRepresentation: START");
            if (auto resourceRepresentation = Cast<UFGResourceNodeRepresentation>( actorRepresentation ))
            {
                if (resourceRepresentation->IsCluster())
                {
                    scope.Cancel();
                    SRM_LOG("AFGActorRepresentationManager::AddRepresentation: END NOT ADDING CLUSTER");
                    return;
                }
            }

            scope(self, actorRepresentation);
            SRM_LOG("AFGActorRepresentationManager::AddRepresentation: END");
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
                    if (!this->ResourceTypeNameByResourceRepresentationType.Contains(resourceType))
                    {
                        this->ResourceTypeNameByResourceRepresentationType.Add(resourceType, nodeRep->GetResourceNode()->GetResourceName());
                    }

                    scope.Override((ERepresentationType)resourceType);
                    return (ERepresentationType)resourceType;
                }
            }

            auto representationType = scope(self);
            return representationType;
        });

    SUBSCRIBE_METHOD(AFGResourceScanner::ShowResourceDescriptorSelectUI,
        [](auto& scope, AFGResourceScanner* self)
        {
            SRM_LOG("AFGResourceScanner::ShowResourceDescriptorSelectUI: START %s", *self->GetName());
            self->mRepresentationLifeSpan = 0.0;
            scope(self);
            SRM_LOG("AFGResourceScanner::ShowResourceDescriptorSelectUI: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGHUD::OnActorRepresentationAdded,
        [](auto& scope, AFGHUD* self, UFGActorRepresentation* actorRepresentation)
        {
            SRM_LOG("AFGHUD::OnActorRepresentationAdded: START %s", *self->GetName());

            if (auto nodeRep = Cast<UFGResourceNodeRepresentation>(actorRepresentation))
            {
                ERepresentationType representationType = nodeRep->GetRepresentationType();
                if (representationType > (ERepresentationType)EResourceRepresentationType::RRT_Default)
                {
                    TArray<FCompassEntry> entriesCopy(self->GetCompassEntries());
                    for (auto& entry : entriesCopy)
                    {
                        if (auto existingNodeRep = Cast<UFGResourceNodeRepresentation>(entry.RepresentingActor))
                        {
                            if (existingNodeRep->GetResourceNode() == nodeRep->GetResourceNode())
                            {
                                existingNodeRep->RemoveActorRepresentation();
                                break;
                            }
                        }
                    }
                }
            }

            scope(self, actorRepresentation);

            SRM_LOG("AFGHUD::OnActorRepresentationAdded: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGHUD::OnActorRepresentationFiltered,
        [&](auto& scope, AFGHUD* self, ERepresentationType type, bool visible)
        {
            SRM_LOG("AFGHUD::OnActorRepresentationFiltered: START %s, %d, %d", *self->GetName(), type, visible);

            if (type > (ERepresentationType)EResourceRepresentationType::RRT_Default)
            {
                for (auto& entry : self->GetCompassEntries())
                {
                    if (auto nodeRep = Cast<UFGResourceNodeRepresentation>(entry.RepresentingActor))
                    {
                        EResourceRepresentationType resRepType;
                        if (TryGetResourceRepresentationType(nodeRep, resRepType) && (ERepresentationType)resRepType == type)
                        {
                            entry.bEnabled = visible;
                        }
                    }
                }
            }
            else
            {
                scope(self, type, visible);
            }

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
        ERepresentationType* repTypePtr = helper.GetLocalVarEnumPtr<ERepresentationType>(TEXT("representationType"));
        if (*repTypePtr > (ERepresentationType)EResourceRepresentationType::RRT_Default)
        {
            // Use the default resource Z order
            *repTypePtr = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK_RETURN(BPW_MapMenuClass, GetGenericClass)
        TObjectPtr<UObject> classValue = *helper.GetOutVariablePtr<FObjectProperty>( TEXT("Class") );
        if (!classValue)
        {
            helper.SetOutVarBool( TEXT("HasGenericClass"), false);
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapMenuClass, ShouldAddToMenu, 64)
        ERepresentationType* repTypePtr = helper.GetLocalVarEnumPtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (*repTypePtr > (ERepresentationType)EResourceRepresentationType::RRT_Default)
        {
            *repTypePtr = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK_RETURN(BPW_MapMenuClass, ShouldAddToMenu)
        auto rep = *helper.GetLocalVarPtr<FObjectProperty>( TEXT("actorRepresentation"));

        if (auto nodeRep = Cast<UFGResourceNodeRepresentation>(rep))
        {
            if (nodeRep->IsCluster())
            {
                SRM_LOG("ShouldAddToMenu: Changing ReturnValue to false because it's a cluster");
                helper.SetOutVarBool(TEXT("ReturnValue"), false);
            }
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapMenuClass, AddActorRepresentationToMenu, 1447)
        auto localTypePtr = helper.GetLocalVarEnumPtr<EResourceRepresentationType>(TEXT("LocalType"));
        if( *localTypePtr > EResourceRepresentationType::RRT_Default )
        {
            helper.SetLocalVarBool(TEXT("K2Node_SwitchEnum_CmpSuccess"), false);
        }
    FINISH_HOOK


    BEGIN_HOOK_RETURN(BPW_MapFilterCategoriesClass, GetCategoryName)
        EResourceRepresentationType resourceType = *helper.GetContextVarEnumPtr<EResourceRepresentationType>(TEXT("mRepresentationType"));
        SRM_LOG("BPW_MapFilterCategories::GetCategoryName: resourceType %d", resourceType);

        if (resourceType > EResourceRepresentationType::RRT_Default)
        {
            FText* nameText = this->ResourceTypeNameByResourceRepresentationType.Find(resourceType);
            if (!nameText)
            {
                return;
            }

            FText* ReturnValuePointer = helper.GetOutVariablePtr<FTextProperty>(TEXT("ReturnValue"));
            *ReturnValuePointer = *nameText;
        }
    FINISH_HOOK

    BEGIN_HOOK_RETURN(BPW_MapFilterCategoriesClass, CanBeSeenOnCompass)
        ERepresentationType repType = *helper.GetLocalVarEnumPtr<ERepresentationType>( TEXT("Index"));
        if( repType == ERepresentationType::RT_Resource || repType > (ERepresentationType)EResourceRepresentationType::RRT_Default )
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

    SRMDebugging::RegisterDebugHooks_Widget_MapTab(Widget_MapTabClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_Map(Widget_MapClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_MapObject(Widget_MapObjectClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_MapCompass_Icon(Widget_MapCompass_IconClass.Get());

    SRMDebugging::RegisterDebugHooks_BPW_MapMenu(BPW_MapMenuClass.Get());
    SRMDebugging::RegisterDebugHooks_BPW_MapFilterCategories(BPW_MapFilterCategoriesClass.Get());
    SRMDebugging::RegisterDebugHooks_BPW_MapFiltersSubCategory(BPW_MapFiltersSubCategoryClass.Get());
    SRMDebugging::RegisterDebugHooks_BPW_MapFilterButton(BPW_MapFilterButtonClass.Get());
}
