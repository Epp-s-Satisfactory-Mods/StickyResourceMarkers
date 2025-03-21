#include "SRMRootInstanceModule.h"

#include "FGActorRepresentation.h"
#include "FGActorRepresentationManager.h"
#include "FGBuildableRadarTower.h"
#include "FGPlayerController.h"
#include "FGPlayerState.h"
#include "FGResourceNodeBase.h"
#include "FGResourceNodeFrackingCore.h"
#include "FGResourceNodeRepresentation.h"
#include "FGResourceScanner.h"
#include "FGHUD.h"
#include "Patching/NativeHookManager.h"
#include "Patching/BlueprintHookManager.h"
#include "Patching/BlueprintHookHelper.h"

#include "AssetRegistryModule.h"
#include "CanvasItem.h"
#include "CanvasPanelSlot.h"
#include "Engine/Canvas.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "FindLast.h"
#include "HorizontalBox.h"
#include "HorizontalBoxSlot.h"
#include "OutputDeviceNull.h"
#include "WidgetBlueprintGeneratedClass.h"

#include "SRMDebugging.h"
#include "SRMHookMacros.h"
#include "SRMLogMacros.h"
#include "SRMRootGameWorldModule.h"

#define LOCTEXT_NAMESPACE "StickyResourceMarkers"


USRMRootGameWorldModule* USRMRootInstanceModule::CurrentGameWorldModule = nullptr;

void USRMRootInstanceModule::DispatchLifecycleEvent(ELifecyclePhase phase)
{
    SRM_LOG("USRMRootInstanceModule::DispatchLifecycleEvent: Phase %d", phase);

    switch (phase)
    {
    case ELifecyclePhase::CONSTRUCTION:
        Initialize();
        break;
    }

    Super::DispatchLifecycleEvent(phase);
}

void USRMRootInstanceModule::SetAllResourcesVisibility(AFGCharacterPlayer* player, EResourceVisibilityLocation location, bool visible)
{
    auto gameWorldModule = this->GetGameWorldModule();
    auto manager = AFGActorRepresentationManager::Get(gameWorldModule->GetWorld());

    SRM_LOG("SetAllResourcesVisibility: Location: %d, Visibile: %d", location, visible);

    for (uint8 representationTypeId = (uint8)this->FirstResourceRepresentationType; representationTypeId <= (uint8)this->LastResourceRepresentationType; ++representationTypeId)
    {
        switch (location)
        {
        case EResourceVisibilityLocation::Compass:
            manager->SetCompassRepresentationTypeFilter(player, (ERepresentationType)representationTypeId, visible);
            break;
        case EResourceVisibilityLocation::Map:
            manager->SetMapRepresentationTypeFilter(player, (ERepresentationType)representationTypeId, visible);
            break;
        }
    }
}

void USRMRootInstanceModule::SetAllResourcesCompassVisibility(AFGCharacterPlayer* player, bool visible)
{
    auto gameWorldModule = this->GetGameWorldModule();
    auto manager = AFGActorRepresentationManager::Get(gameWorldModule->GetWorld());

    SRM_LOG("SetAllResourcesCompassVisibility: Visibile: %d", visible);

    for (uint8 representationTypeId = (uint8)this->FirstResourceRepresentationType; representationTypeId <= (uint8)this->LastResourceRepresentationType; ++representationTypeId)
    {
        manager->SetCompassRepresentationTypeFilter(player, (ERepresentationType)representationTypeId, visible);
    }
}

void USRMRootInstanceModule::SetAllResourcesMapVisibility(AFGCharacterPlayer* player, bool visible)
{
    auto gameWorldModule = this->GetGameWorldModule();
    auto manager = AFGActorRepresentationManager::Get(gameWorldModule->GetWorld());

    SRM_LOG("SetAllResourcesMapVisibility: Visibile: %d", visible);

    for (uint8 representationTypeId = (uint8)this->FirstResourceRepresentationType; representationTypeId <= (uint8)this->LastResourceRepresentationType; ++representationTypeId)
    {
        manager->SetMapRepresentationTypeFilter(player, (ERepresentationType)representationTypeId, visible);
    }
}

bool USRMRootInstanceModule::TryGetResourceRepresentationType(TSubclassOf<UFGResourceDescriptor> resourceDescriptor, ERepresentationType& resourceRepresentationType)
{
    auto resourceDescriptorName = resourceDescriptor->GetFName();
    auto representationType = this->ResourceRepresentationTypeByDescriptorName.Find(resourceDescriptorName);

    if (representationType)
    {
        resourceRepresentationType = (ERepresentationType)*representationType;
        return true;
    }

    return false;
}

bool USRMRootInstanceModule::TryGetResourceRepresentationType(const AFGResourceNodeBase* resourceNode, ERepresentationType& resourceRepresentationType)
{
    if (!resourceNode)
    {
        return false;
    }

    auto resourceDescriptor = resourceNode->GetResourceClass();
    return TryGetResourceRepresentationType(resourceDescriptor, resourceRepresentationType);
}

bool USRMRootInstanceModule::TryGetResourceRepresentationType(const UFGResourceNodeRepresentation* nodeRep, ERepresentationType& resourceRepresentationType)
{
    if (nodeRep->IsCluster())
    {
        // We won't end up adding clusters to the compass/map/menu, so just use the default
        return false;
    }

    return TryGetResourceRepresentationType(nodeRep->GetResourceNode(), resourceRepresentationType);
}

void USRMRootInstanceModule::Initialize()
{
    SRM_LOG("USRMRootInstanceModule::Initialize");

    if (WITH_EDITOR)
    {
        SRM_LOG("USRMRootInstanceModule::Initialize: Not initializing anything because WITH_EDITOR is true!");
        return;
    }

    SRM_LOG("Expanding ERepresentationType enum...");

    // We need to expand the ERepresentationType enum to contain our values or they will not be serialized properly by UE
    UEnum* repTypeEnum = StaticEnum<ERepresentationType>();
    int numEnums = repTypeEnum->NumEnums();
    bool hasExistingMax = repTypeEnum->ContainsExistingMax();
    if (hasExistingMax)
    {
        numEnums--;
    }

    TArray<TPair<FName, int64>> allEnums;
    int largestExistingValue = -1;
    for (int i = 0; i < numEnums; ++i)
    {
        auto name = repTypeEnum->GetNameByIndex(i);
        auto value = repTypeEnum->GetValueByIndex(i);
        largestExistingValue = FMath::Max(value, largestExistingValue);
        allEnums.Emplace(TPair<FName, int64>(name, value));
    }

    auto nextValue = largestExistingValue + 1;
    this->FirstResourceRepresentationType = (ERepresentationType)(nextValue);

    // Map the resource descriptor names to their ResourceRepresentationTypes and prepare add them to the enum list for redefining the num.
    // We use FNames because they are interned and so lookups shouldn't need to compare the whole string
#define SETUP_RESOURCE_REPRESENTATION_TYPE( ENUM_VALUE_NAME, DESCRIPTOR_NAME ) \
    ResourceRepresentationTypeByDescriptorName.Add(#DESCRIPTOR_NAME, (ERepresentationType)nextValue); \
    allEnums.Emplace(TPair<FName, int64>("ERepresentationType::"#ENUM_VALUE_NAME, nextValue++));

    // I had to manually discover the descriptor names so any new ones will need to be manually discovered/added
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_Coal, Desc_Coal_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_Geyser, Desc_Geyser_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_LiquidOil, Desc_LiquidOil_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_NitrogenGas, Desc_NitrogenGas_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_OreBauxite, Desc_OreBauxite_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_OreCopper, Desc_OreCopper_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_OreGold, Desc_OreGold_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_OreIron, Desc_OreIron_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_OreUranium, Desc_OreUranium_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_RawQuart, Desc_RawQuartz_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_SAM, Desc_SAM_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_Stone, Desc_Stone_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_Sulfur, Desc_Sulfur_C);
    SETUP_RESOURCE_REPRESENTATION_TYPE(RT_Water, Desc_Water_C);

    // Set all the values in ERepresentationType
    repTypeEnum->SetEnums(allEnums, repTypeEnum->GetCppForm(), EEnumFlags::None, hasExistingMax);

    this->LastResourceRepresentationType = (ERepresentationType)(nextValue - 1); // Subtract one since the macro always increments

    SRM_LOG("Registering hooks...");

    RegisterDebugHooks();

    if (SRM_DEBUGGING_ENABLED && !SRM_DEBUGGING_REGISTER_MOD_HOOKS)
    {
        SRM_LOG("USRMRootInstanceModule::Initialize: Not registering mod hooks because SRM_DEBUGGING_ENABLED is 1 and SRM_DEBUGGING_REGISTER_MOD_HOOKS is 0!");
        return;
    }

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
        [&](auto& scope, AFGResourceNodeBase* self)
        {
            SRM_LOG("AFGResourceNodeBase::ScanResourceNodeScan_Server: START");

            auto gameWorldModule = this->GetGameWorldModule();

            // Make sure we don't operate on modded/unknown resource types
            ERepresentationType representationType;
            bool isKnownResourceType = TryGetResourceRepresentationType(self, representationType);
            if (isKnownResourceType)
            {
                if (gameWorldModule->IsNodeCurrentlyRepresented(self))
                {
                    scope.Cancel();
                    SRM_LOG("AFGResourceNodeBase::ScanResourceNodeScan_Server: END (CANCELED - NODE %s ALREADY REPRESENTED)", *self->GetName());
                    return;
                }
            }

            scope(self);

            if (isKnownResourceType)
            {
                gameWorldModule->SetNodeRepresented(self);
            }
            SRM_LOG("AFGResourceNodeBase::ScanResourceNodeScan_Server: END");
        });

    SUBSCRIBE_UOBJECT_METHOD(AFGResourceScanner, ScanReleased,
        [&](auto& scope, AFGResourceScanner* self)
        {
            auto scanningFor = self->mResourceDescriptorToScanFor;
            SRM_LOG("AFGResourceScanner::ScanReleased: START mResourceDescriptorToScanFor: %s", *(scanningFor.Get() ? scanningFor->GetName() : TEXT("null")));
            SRM_LOG("AFGResourceScanner::ScanReleased: AFGResourceScanner: %s. Owner: %s", *self->GetName(), *self->GetOwner()->GetName());
            SRM_LOG("AFGResourceScanner::ScanReleased: PlayerController %s",
                *((Cast<AFGCharacterPlayer>(self->GetOwner())->GetFGPlayerController() == nullptr) ?
                TEXT("null") :
                Cast<AFGCharacterPlayer>(self->GetOwner())->GetFGPlayerController()->GetName()));

            if (scanningFor == nullptr)
            {
                SRM_LOG("AFGResourceScanner::ScanReleased: mResourceDescriptorToScanFor is null. Calling scope and returning.");
                scope(self);
                return;
            }

            // When we scan for a resource, we may need to set it to visible on the compass and/or map depending on user settings.
            ERepresentationType resourceRepresentationType;
            if (TryGetResourceRepresentationType(scanningFor, resourceRepresentationType))
            {
                auto player = Cast<AFGCharacterPlayer>(self->GetOwner());
                auto gameWorldModule = this->GetGameWorldModule();
                auto manager = AFGActorRepresentationManager::Get(gameWorldModule->GetWorld());
                if (gameWorldModule->GetScanningUnhidesOnCompass())
                {
                    manager->SetCompassRepresentationTypeFilter(player, resourceRepresentationType, true);
                }

                if (gameWorldModule->GetScanningUnhidesOnMap())
                {
                    manager->SetMapRepresentationTypeFilter(player, resourceRepresentationType, true);
                }
            }

            scope(self);

            SRM_LOG("AFGResourceScanner::ScanReleased: END");
        });

    SUBSCRIBE_METHOD(AFGResourceScanner::CreateResourceNodeRepresentations,
        [&](auto& scope, AFGResourceScanner* self, const FNodeClusterData& cluster)
        {
            SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: START %d nodes", cluster.Nodes.Num());

            auto gameWorldModule = this->GetGameWorldModule();

            // Make sure we don't operate on modded/unknown resource types
            ERepresentationType representationType;
            if (!TryGetResourceRepresentationType(cluster.ResourceDescriptor, representationType))
            {
                scope(self, cluster);
                SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: END (DEFAULT - Was not a known resource representation type)");
            }

            // Cluster representations don't have a resource descriptor readily available, so it would take
            // workarounds to support them correctly. Plus, because they don't have a descriptor, the map menu already
            // has a bug where it adds empty lines to the menu on scan.  Overall, clusters icons add so little value
            // (and, to me, negative value) that they aren't worth supporting.
            // This splits clusters into individual nodes so that we don't have to deal with clusters at all anywhere.
            for (auto node : cluster.Nodes)
            {
                if (gameWorldModule->IsNodeCurrentlyRepresented(node))
                {
                    continue;
                }

                gameWorldModule->Local_CreateRepresentation_Server(node);
                gameWorldModule->SetNodeRepresented(node);
            }

            scope.Cancel();
            SRM_LOG("AFGResourceScanner::CreateResourceNodeRepresentations: END (CANCELED!)");
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::UpdateNodeRepresentation,
        [&](auto& scope, AFGResourceNodeBase* self)
        {
            SRM_LOG("AFGResourceNodeBase::UpdateNodeRepresentation: START %s, FName: %s", *self->GetName(), *self->GetFName().ToString());

            auto gameWorldModule = this->GetGameWorldModule();
            if (gameWorldModule->IsGameInitializing)
            {
                // Make sure we don't operate on modded/unknown resource types
                ERepresentationType representationType;
                if (TryGetResourceRepresentationType(self, representationType))
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
            }

            scope(self);
            SRM_LOG("AFGResourceNodeBase::UpdateNodeRepresentation: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD( UFGResourceNodeRepresentation::SetupResourceNodeRepresentation,
        [&](auto& scope, UFGResourceNodeRepresentation* self, class AFGResourceNodeBase* resourceNode)
        {
            SRM_LOG("UFGResourceNodeRepresentation::SetupResourceNodeRepresentation: START");

            // Here, we make sure we have the resource type name cached for this resource type. I tried multiple different methods of
            // constructing this map on game initialization but the available methods of getting all item descriptors all crash when
            // you attempt to access their item names at any point in initialization.
            ERepresentationType representationType;
            bool isKnownResourceType = TryGetResourceRepresentationType(resourceNode, representationType);
            if (isKnownResourceType)
            {
                if (!this->ResourceTypeNameByResourceRepresentationType.Contains(representationType))
                {
                    this->ResourceTypeNameByResourceRepresentationType.Add(representationType, resourceNode->GetResourceName());
                }
            }

            scope(self, resourceNode);

            // Since we only add server-side and that's never from the resource scanner, then it's being added with the assumption that it's only going on the map.
            // Because we make all resource markers available to the compass, we need to finish setting it up for the compass.
            if (isKnownResourceType)
            {
                self->mRepresentationColor = FLinearColor::White;
                self->mShouldShowInCompass = true;
                //self->mCompassViewDistance = ECompassViewDistance::CVD_Always;
            }

            SRM_LOG("UFGResourceNodeRepresentation::SetupResourceNodeRepresentation END");
        });

    SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::GetCompassViewDistance,
        GetMutableDefault<UFGActorRepresentation>(),
        [&](auto& scope, const UFGActorRepresentation* self)
        {
            SRM_LOG("UFGActorRepresentation::GetCompassViewDistance START %s", *self->GetName());
            //SRMDebugging::DumpRepresentation("UFGActorRepresentation::GetCompassViewDistance START", self);

            if (auto nodeRep = Cast<UFGResourceNodeRepresentation>(self))
            {
                ERepresentationType representationType;
                if (TryGetResourceRepresentationType(nodeRep, representationType))
                {
                    auto viewDistance = this->GetGameWorldModule()->GetResourceCompassViewDistance();
                    scope.Override(viewDistance);
                    SRM_LOG("UFGActorRepresentation::GetCompassViewDistance END (OVERRIDE WITH %s)", *StaticEnum<ECompassViewDistance>()->GetNameStringByValue((int64)viewDistance));
                    return viewDistance;
                }
            }

            return scope(self);
            SRM_LOG("UFGActorRepresentation::GetCompassViewDistance END");
        });

    SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::GetRepresentationType,
        GetMutableDefault<UFGActorRepresentation>(),
        [&](auto& scope, const UFGActorRepresentation* self)
        {
            if (auto nodeRep = Cast<UFGResourceNodeRepresentation>(self))
            {
                ERepresentationType representationType;
                if (TryGetResourceRepresentationType(nodeRep, representationType))
                {
                    scope.Override(representationType);
                    return representationType;
                }
            }

            return scope(self);
        });

    SRM_LOG("Native hooks registered...");

    if (IsRunningDedicatedServer())
    {
        SRM_LOG("On dedicated server. Not attempting to register UI blueprint hooks since they won't exist...");
        return;
    }

    // Make sure the blueprint types we're going to modify are loaded
    SRMResourceVisibilityButtonClass.LoadSynchronous();

    Widget_MapContainerClass.LoadSynchronous();
    Widget_MapObjectClass.LoadSynchronous();
    Widget_MapClass.LoadSynchronous();
    BPW_MapFilterCategoriesClass.LoadSynchronous();
    BPW_MapFilterButtonClass.LoadSynchronous();
    Widget_MapCompass_IconClass.LoadSynchronous();


    AddButtonsToMapScreen();

    // These blueprint hooks all hook very specific locations in their blueprint functions to trick the map and compass UIs into treating them like
    // they have RepresentationType RT_Resource and/or to add the bits of custom behavior (like putting them in different categories and making them
    // filterable on the compass). If an underlying blueprint function changes even a little, there's a good chance these will have to be updated.
    // Look in the HookSnapshots folder in the root of the repo for a README that explains how to find the differences.

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS

    BEGIN_HOOK(Widget_MapCompass_IconClass, UpdateActor, 108)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (this->IsResourceRepresentationType(*representationType))
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(Widget_MapObjectClass, mShowActorDetails, 59)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (this->IsResourceRepresentationType(*representationType))
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK_START(Widget_MapClass, GetZOrderForType)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("representationType"));
        if (this->IsResourceRepresentationType(*representationType))
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapMenuClass, ShouldAddToMenu, 64)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("CallFunc_GetRepresentationType_ReturnValue"));
        if (this->IsResourceRepresentationType(*representationType))
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapMenuClass, AddActorRepresentationToMenu, 1447)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("LocalType"));
        if (this->IsResourceRepresentationType(*representationType))
        {
            localHelper->SetBoolVariable(TEXT("K2Node_SwitchEnum_CmpSuccess"), false);
        }
    FINISH_HOOK

    BEGIN_HOOK_RETURN(BPW_MapFilterCategoriesClass, GetCategoryName)
        auto contextHelper = helper.GetContextVariableHelper();
        ERepresentationType representationType = *contextHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("mRepresentationType"));
        if (this->IsResourceRepresentationType(representationType))
        {
            FText* nameText = this->ResourceTypeNameByResourceRepresentationType.Find(representationType);
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
        if(this->IsResourceRepresentationType(representationType))
        {
            auto outHelper = helper.GetOutVariableHelper();
            outHelper->SetBoolVariable(TEXT("ReturnValue"), true);
        }
    FINISH_HOOK

    BEGIN_HOOK(BPW_MapFilterButtonClass, SetActorRepresentation, 378)
        auto localHelper = helper.GetLocalVariableHelper();
        ERepresentationType* representationType = localHelper->GetEnumVariablePtr<ERepresentationType>(TEXT("Temp_byte_Variable"));
        if (this->IsResourceRepresentationType(*representationType))
        {
            *representationType = ERepresentationType::RT_Resource;
        }
    FINISH_HOOK

    SRM_LOG("Blueprint hooks registered...");
}

void USRMRootInstanceModule::AddButtonsToMapScreen()
{
    class UPanelWidgetAccessor : UPanelWidget
    {
    public:
        static UClass* GetPanelSlotClass(const UPanelWidget* PanelWidget) {
            return static_cast<const UPanelWidgetAccessor*>(PanelWidget)->GetSlotClass();
        }

        static TArray<UPanelSlot*>& GetPanelSlots(UPanelWidget* PanelWidget) {
            return static_cast<UPanelWidgetAccessor*>(PanelWidget)->Slots;
        }
        UPanelWidgetAccessor() = delete;
    };

    const auto* mapContainerClass = Cast<UWidgetBlueprintGeneratedClass>(Widget_MapContainerClass.Get());
    UWidgetTree* mapContainerWidgetTree = mapContainerClass->GetWidgetTreeArchetype();
    UWidget* showHideMapMenuButton = mapContainerWidgetTree->FindWidget("ShowHideButton");
    checkf(showHideMapMenuButton, TEXT("Could not find button that shows/hides the map menu!"));

    int32 childIndex;
    UPanelWidget* showHideMapMenuContainingPanel = UWidgetTree::FindWidgetParent(showHideMapMenuButton, childIndex);

    UHorizontalBox* newHBox = NewObject<UHorizontalBox>(mapContainerWidgetTree, UHorizontalBox::StaticClass(), "ShowHideResourcesHBox", RF_Transient);

    auto* hBoxPanelSlot = NewObject<UCanvasPanelSlot>(showHideMapMenuContainingPanel, UCanvasPanelSlot::StaticClass(), NAME_None, RF_Transient);
    hBoxPanelSlot->Content = newHBox;
    hBoxPanelSlot->Parent = showHideMapMenuContainingPanel;
    hBoxPanelSlot->SetPosition({ 6, 6 });
    hBoxPanelSlot->SetAutoSize(true);

    newHBox->Slot = hBoxPanelSlot;

    // Move the show/hide map menu button to the new hbox
    newHBox->AddChild(showHideMapMenuButton);

    UWidget* hideAllResourcesOnCompassButton = CreateResourceVisibilityButton(newHBox,
        TEXT("SRMHideAllOnCompass"),
        LOCTEXT("SRMHideAllOnCompass", "Hide All Resources On Compass"),
        EResourceVisibilityLocation::Compass,
        false,
        showHideMapMenuButton);
    Cast<UHorizontalBoxSlot>(newHBox->AddChild(hideAllResourcesOnCompassButton))->SetPadding({ 10, 0, 0, 0 });

    UWidget* showAllResourcesOnCompassButton = CreateResourceVisibilityButton(newHBox,
        TEXT("SRMShowAllOnCompass"),
        LOCTEXT("SRMShowAllOnCompass", "Show All Resources On Compass"),
        EResourceVisibilityLocation::Compass,
        true,
        showHideMapMenuButton);
    Cast<UHorizontalBoxSlot>(newHBox->AddChild(showAllResourcesOnCompassButton))->SetPadding({ 10, 0, 0, 0 });

    UWidget* hideAllResourcesOnMapButton = CreateResourceVisibilityButton(newHBox,
        TEXT("SRMHideAllOnMap"),
        LOCTEXT("SRMHideAllOnMap", "Hide All Resources On Map"),
        EResourceVisibilityLocation::Map,
        false,
        showHideMapMenuButton);
    Cast<UHorizontalBoxSlot>(newHBox->AddChild(hideAllResourcesOnMapButton))->SetPadding({ 10, 0, 0, 0 });

    UWidget* showAllResourcesOnMapButton = CreateResourceVisibilityButton(newHBox,
        TEXT("SRMShowAllOnMap"),
        LOCTEXT("SRMShowAllOnMap", "Show All Resources On Map"),
        EResourceVisibilityLocation::Map,
        true,
        showHideMapMenuButton);
    Cast<UHorizontalBoxSlot>(newHBox->AddChild(showAllResourcesOnMapButton))->SetPadding({ 10, 0, 0, 0 });

    TArray<UPanelSlot*>& MutablePanelSlots = UPanelWidgetAccessor::GetPanelSlots(showHideMapMenuContainingPanel);
    MutablePanelSlots.Insert(hBoxPanelSlot, childIndex);
}

UWidget* USRMRootInstanceModule::CreateResourceVisibilityButton(
    UObject* outer,
    FName name,
    FText label,
    EResourceVisibilityLocation visibilityLocation,
    bool visible,
    UWidget* templateWidget)
{
    auto buttonClass = SRMResourceVisibilityButtonClass.Get();

    UWidget* button = NewObject<UWidget>(outer, buttonClass, name, RF_Transient, templateWidget);

    FProperty* mTextProperty = button->GetClass()->FindPropertyByName("mText");
    checkf(mTextProperty, TEXT("Did not find mText property"));
    FText* mTextPtr = mTextProperty->ContainerPtrToValuePtr<FText>(button);
    *mTextPtr = label;

    FProperty* mVisibilityLocationProperty = button->GetClass()->FindPropertyByName("mVisibilityLocation");
    checkf(mVisibilityLocationProperty, TEXT("Did not find mVisibilityLocation property"));
    EResourceVisibilityLocation* mVisibilityLocationPtr = mVisibilityLocationProperty->ContainerPtrToValuePtr<EResourceVisibilityLocation>(button);
    *mVisibilityLocationPtr = visibilityLocation;

    FBoolProperty* mVisibilityToSetProperty = CastField<FBoolProperty>(button->GetClass()->FindPropertyByName("mVisibilityToSet"));
    checkf(mVisibilityToSetProperty, TEXT("Did not find mVisibilityToSet property"));
    void* mVisibilityToSetPtr = mVisibilityToSetProperty->ContainerPtrToValuePtr<void>(button);
    mVisibilityToSetProperty->SetPropertyValue(mVisibilityToSetPtr, visible);

    return button;
}

void USRMRootInstanceModule::RegisterDebugHooks()
{
    if (!SRM_DEBUGGING_ENABLED) return;

    SRMDebugging::RegisterNativeDebugHooks();

    if (IsRunningDedicatedServer())
    {
        // Map UI blueprints don't exist on a dedicated server build
        return;
    }

    SRMDebugging::RegisterDebugHooks_Widget_MapContainer(Widget_MapContainerClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_MapObject(Widget_MapObjectClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_Map(Widget_MapClass.Get());
    SRMDebugging::RegisterDebugHooks_BPW_MapFilterCategories(BPW_MapFilterCategoriesClass.Get());
    SRMDebugging::RegisterDebugHooks_BPW_MapFilterButton(BPW_MapFilterButtonClass.Get());
    SRMDebugging::RegisterDebugHooks_Widget_MapCompass_Icon(Widget_MapCompass_IconClass.Get());

    SRMDebugging::RegisterDebugHooks_Widget_MapTab(Widget_MapTabClass.Get());
    SRMDebugging::RegisterDebugHooks_BPW_MapMenu(BPW_MapMenuClass.Get());
}

#undef LOCTEXT_NAMESPACE