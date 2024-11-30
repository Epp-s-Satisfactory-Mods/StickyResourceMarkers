#include "SRMDebugging.h"

#include "FGActorRepresentationManager.h"
#include "FGBuildableRadarTower.h"
#include "FGResourceNode.h"
#include "FGResourceNodeFrackingCore.h"
#include "FGResourceNodeFrackingSatellite.h"
#include "FGResourceNodeRepresentation.h"
#include "FGResourceScanner.h"
#include "FGHUD.h"
#include "SRMHookMacros.h"
#include "SRMLogMacros.h"
#include "Patching/NativeHookManager.h"
#include "Widget.h"
#include "Struct_ActorRep.h"

void SRMDebugging::DumpWidget(FString prefix, const UWidget* widget)
{
    SRM_LOG("%s: Dumping UWidget", *prefix);
    if (!widget)
    {
        SRM_LOG("%s: UWidget is null", *prefix);
    }

    //SRM_LOG("widget: %x",widget);
    SRM_LOG("%s: Widget: %x.", *prefix, widget->GetClass());//, * widget->GetClass()->GetName());
}

void SRMDebugging::DumpResourceNode(FString prefix, const AFGResourceNodeBase* res)
{
    if (!res)
    {
        SRM_LOG("%s AFGResourceNodeBase: null", *prefix);
        return;
    }

    SRM_LOG("%s AFGResourceNodeBase: %s (%s) at %s", *prefix, *res->GetName(), *res->GetClass()->GetName(), *res->GetActorLocation().ToString());
    auto nestedPrefix = prefix + "\t";
    SRM_LOG("%s IsOccupied: %d", *nestedPrefix, res->IsOccupied());
    SRM_LOG("%s CanBecomeOccupied: %d", *nestedPrefix, res->CanBecomeOccupied());
    SRM_LOG("%s GetResourceClass: %s", *nestedPrefix, *res->GetResourceClass()->GetName());

    if (auto f = Cast<AFGResourceNodeFrackingCore>(res))
    {
        SRM_LOG("%s Is FrackingCore!", *nestedPrefix);
        SRM_LOG("%s IsAllSatellitesOccupied: %d", *nestedPrefix, f->IsAllSatellitesOccupied());
        int32 occupiedSatellites;
        SRM_LOG("%s GetNumOccupiedSatellites Returns: %d", *nestedPrefix, f->GetNumOccupiedSatellites(occupiedSatellites));
        SRM_LOG("%s GetNumOccupiedSatellites Output Param: %d", *nestedPrefix, occupiedSatellites);

        TArray<  AFGResourceNodeFrackingSatellite* > satellites;
        f->GetSatellites(satellites);
        SRM_LOG("%s Satellites: %d", *nestedPrefix, satellites.Num());

        auto satPrefix = nestedPrefix + "\t";
        int i = 0;
        for (auto s : satellites)
        {
            SRM_LOG("%s Satellite[%d]", *nestedPrefix, i);
            SRM_LOG("%s CanPlaceResourceExtractor: %d", *satPrefix, s->CanPlaceResourceExtractor());
            SRM_LOG("%s GetState: %d", *satPrefix, s->GetState());
            ++i;
        }
    }
}

void SRMDebugging::DumpRepresentation(FString prefix, const UFGActorRepresentation* rep)
{

    if (!rep)
    {
        SRM_LOG("%s UFGActorRepresentation: null", *prefix);
        return;
    }

    SRM_LOG("%s UFGActorRepresentation: %s", *prefix, *rep->GetName());

    auto nestedPrefix = prefix + "\t";
    SRM_LOG("%s Representation Text: %s", *nestedPrefix, *rep->GetRepresentationText().ToString());
    SRM_LOG("%s GetShouldShowInCompass: %d", *nestedPrefix, rep->GetShouldShowInCompass());
    SRM_LOG("%s mCachedShouldShowInCompass: %d", *nestedPrefix, rep->mCachedShouldShowInCompass);
    SRM_LOG("%s GetShouldShowOnMap: %d", *nestedPrefix, rep->GetShouldShowOnMap());
    SRM_LOG("%s mCachedShouldShowOnMap: %d", *nestedPrefix, rep->mCachedShouldShowOnMap);
    SRM_LOG("%s GetCompassViewDistance: %d", *nestedPrefix, rep->GetCompassViewDistance());
    SRM_LOG("%s mBackgroundIsPrimaryColor: %d", *nestedPrefix, rep->mBackgroundIsPrimaryColor);
    //SRM_LOG("%s GetRepresentationCompassMaterial: %s", *nestedPrefix, *rep->GetRepresentationCompassMaterial()->GetName());
    SRM_LOG("%s GetCompassHeightAlignment: %f", *nestedPrefix, rep->GetCompassHeightAlignment());
    SRM_LOG("%s mRepresentationCompassMaterial: %s", *nestedPrefix, *(rep->mRepresentationCompassMaterial == nullptr ? TEXT("null") : rep->mRepresentationCompassMaterial->GetName()));
    SRM_LOG("%s GetRepresentationTexture: %s", *nestedPrefix, *rep->GetRepresentationTexture()->GetName());
    SRM_LOG("%s mRepresentationTexture: %s", *nestedPrefix, *(rep->mRepresentationTexture == nullptr ? TEXT("null") : rep->mRepresentationTexture->GetName()));
    SRM_LOG("%s mRepresentationColor: %s", *nestedPrefix, *rep->mRepresentationColor.ToString());
    SRM_LOG("%s IsImportantCompassRepresentation: %d", *nestedPrefix, rep->IsImportantCompassRepresentation());
    SRM_LOG("%s NeedsDynamicCompassRepresentationText: %d", *nestedPrefix, rep->NeedsDynamicCompassRepresentationText());
    SRM_LOG("%s NeedsDynamicCompassRepresentationScale: %d", *nestedPrefix, rep->NeedsDynamicCompassRepresentationScale());
    SRM_LOG("%s GetRepresentationCompassEffectMaterial: %s", *nestedPrefix, *(rep->GetRepresentationCompassEffectMaterial() == nullptr ? TEXT("null") : rep->GetRepresentationCompassEffectMaterial()->GetName()));
    SRM_LOG("%s GetRepresentationCompassEffectSize: %s", *nestedPrefix, *rep->GetRepresentationCompassEffectSize().ToString());

    if (auto res = Cast<UFGResourceNodeRepresentation>(rep))
    {
        SRM_LOG("%s Resource Node Representation:", *nestedPrefix);
        SRM_LOG("%s\t IsCluster: %d", *nestedPrefix, res->IsCluster());
        SRM_LOG("%s\t mIsCluster: %d", *nestedPrefix, res->mIsCluster);
        SRM_LOG("%s\t IsOccupied: %d", *nestedPrefix, res->IsOccupied());
        SRM_LOG("%s\t mIsOccupied: %d", *nestedPrefix, res->mIsOccupied);
        SRM_LOG("%s\t mIsScannerOwned: %d", *nestedPrefix, res->mIsScannerOwned);
        SRM_LOG("%s\t mScanCount: %d", *nestedPrefix, res->mScanCount);
        SRM_LOG("%s\t RepresentationType: %d", *nestedPrefix, res->GetRepresentationType());
        DumpResourceNode(nestedPrefix + "\t", res->GetResourceNode());
    }
}

void SRMDebugging::DumpFStruct_ActorRep(FString prefix, FStruct_ActorRep* actorRep)
{
    if (!actorRep)
    {
        SRM_LOG("%s FStruct_ActorRep: null", *prefix);
    }

    SRM_LOG("%s FStruct_ActorRep", *prefix);
    auto nestedPrefix = prefix + "\t";
    SRM_LOG("%s Count %d", *nestedPrefix, actorRep->ActorRepresentations.Num());

    nestedPrefix = prefix + "\t";
    int i = 0;
    for (UFGActorRepresentation* rep : actorRep->ActorRepresentations)
    {
        DumpRepresentation( FString::Printf( TEXT("%s[%d]"), *nestedPrefix, i ), rep);
        ++i;
    }
}
void SRMDebugging::DumpColor(FString prefix, FLinearColor color)
{
    SRM_LOG("%s FLinearColor: %s", *prefix, *color.ToString());
}

void SRMDebugging::DumpUObjectPtr(FString prefix, UObject** object)
{
    DumpUObject(prefix, *object);
}

void SRMDebugging::DumpUObject(FString prefix, UObject* object)
{
    if (!object )
    {
        SRM_LOG("%s UObject: null", *prefix);
        return;
    }

    if (auto rep = Cast<UFGActorRepresentation>(object))
    {
        DumpRepresentation(prefix, rep);
    }
    else
    {
        SRM_LOG("%s %s (%s)", *prefix, *object->GetName(), *object->GetClass()->GetName());
    }
}

void SRMDebugging::DumpCompassEntry(FString prefix, FCompassEntry& compassEntry, int* indexPtr )
{
    FString indexStr = indexPtr == nullptr ? TEXT("") : FString::Printf(TEXT("[%d]"), *indexPtr);

    SRM_LOG("%s FCompassEntry%s: %s at %x", *prefix, *indexStr, *compassEntry.Text.ToString(), &compassEntry);
    auto nestedPrefix = prefix + "\t";
    SRM_LOG("%s bEnabled: %d", *nestedPrefix, compassEntry.bEnabled);
    SRM_LOG("%s bCanShowName: %d", *nestedPrefix, compassEntry.bCanShowName);
    SRM_LOG("%s bShouldShowName: %d", *nestedPrefix, compassEntry.bShouldShowName);
    SRM_LOG("%s bIsFilteredOut: %d", *nestedPrefix, compassEntry.bIsFilteredOut);
    SRM_LOG("%s bImportantEntry: %d", *nestedPrefix, compassEntry.bImportantEntry);
    DumpRepresentation(nestedPrefix + " RepresentingActor:", compassEntry.RepresentingActor);
}

void SRMDebugging::DumpCompassEntries(FString prefix, TArray<FCompassEntry>& compassEntries)
{
    SRM_LOG("%s CompassEntries Count: %d", *prefix, compassEntries.Num());

    auto nestedPrefix = prefix + "\t";
    int i = 0;
    for (auto& entry : compassEntries)
    {
        DumpCompassEntry(nestedPrefix, entry, &i);
        ++i;
    }
}

void SRMDebugging::RegisterNativeDebugHooks()
{
    if (!SRM_DEBUGGING_TRACE_ALL_NATIVE_HOOKS) return;

    SUBSCRIBE_METHOD(AFGActorRepresentationManager::CreateAndAddNewRepresentation,
        [](auto& scope, AFGActorRepresentationManager* self, AActor* realActor, const bool isLocal = false, TSubclassOf<UFGActorRepresentation> representationClass = nullptr)
        {
            SRM_LOG("AFGActorRepresentationManager::CreateAndAddNewRepresentation: START Actor: %s, isLocal: %d, RepresentaionClass: %s",
                *realActor->GetName(),
                isLocal,
                representationClass == nullptr ? L"null" : *representationClass.Get()->GetName());

            scope(self, realActor, isLocal, representationClass);

            SRM_LOG("AFGActorRepresentationManager::CreateAndAddNewRepresentation: END");
        });

    SUBSCRIBE_METHOD(AFGActorRepresentationManager::UpdateRepresentationOfActor,
        [](auto& scope, AFGActorRepresentationManager* self, AActor* realActor)
        {
            SRM_LOG("AFGActorRepresentationManager::UpdateRepresentationOfActor: START: Actor: %s", *realActor->GetName());
            scope(self, realActor);
            SRM_LOG("AFGActorRepresentationManager::UpdateRepresentationOfActor: END");
        });

    SUBSCRIBE_METHOD(AFGActorRepresentationManager::UpdateRepresentation,
        [](auto& scope, AFGActorRepresentationManager* self, UFGActorRepresentation* actorRepresentation)
        {
            DumpRepresentation("AFGActorRepresentationManager::UpdateRepresentation: START", actorRepresentation);
            scope(self, actorRepresentation);
            SRM_LOG("AFGActorRepresentationManager::UpdateRepresentation: END");
        });

    SUBSCRIBE_METHOD(AFGActorRepresentationManager::RemoveRepresentationOfActor,
        [](auto& scope, AFGActorRepresentationManager* self, AActor* realActor)
        {
            SRM_LOG("AFGActorRepresentationManager::RemoveRepresentationOfActor: START: Actor: %s", *realActor->GetName());
            scope(self, realActor);
            SRM_LOG("AFGActorRepresentationManager::RemoveRepresentationOfActor: END");
        });

    // Causes crashes?
    //SUBSCRIBE_METHOD(AFGActorRepresentationManager::RemoveRepresentation,
    //    [](auto& scope, AFGActorRepresentationManager* self, UFGActorRepresentation* actorRepresentation)
    //    {
    //        DumpRepresentation("AFGActorRepresentationManager::RemoveRepresentation: START", actorRepresentation);
    //        scope(self, actorRepresentation);
    //        SRM_LOG("AFGActorRepresentationManager::RemoveRepresentation: END");
    //    });

    SUBSCRIBE_METHOD(AFGActorRepresentationManager::FindResourceNodeRepresentation,
        [](auto& scope, AFGActorRepresentationManager* self, AFGResourceNodeBase* resourceNode)
        {
            SRM_LOG("AFGActorRepresentationManager::FindResourceNodeRepresentation: Resource node actor: %s", *resourceNode->GetName());
            auto rep = scope(self, resourceNode);
            SRM_LOG("AFGActorRepresentationManager::FindResourceNodeRepresentation: Representation: %s", rep == nullptr ? L"null" : *rep->GetName());
            return rep;
        });

    SUBSCRIBE_METHOD(AFGActorRepresentationManager::SetMapRepresentationTypeFilter,
        [](auto& scope, AFGActorRepresentationManager* self, class APawn* owningPlayerPawn, ERepresentationType type, bool visible)
        {
            SRM_LOG("AFGActorRepresentationManager::SetMapRepresentationTypeFilter: START Owning player: %s. Rep type: %d, Visible; %d", *owningPlayerPawn->GetName(), type, visible);
            scope(self, owningPlayerPawn, type, visible);
            SRM_LOG("AFGActorRepresentationManager::SetMapRepresentationTypeFilter: END");
        });

    SUBSCRIBE_METHOD(AFGActorRepresentationManager::SetCompassRepresentationTypeFilter,
        [](auto& scope, AFGActorRepresentationManager* self, class APawn* owningPlayerPawn, ERepresentationType type, bool visible)
        {
            SRM_LOG("AFGActorRepresentationManager::SetCompassRepresentationTypeFilter: START Owning player: %s. Rep type: %d, Visible; %d", *owningPlayerPawn->GetName(), type, visible);
            scope(self, owningPlayerPawn, type, visible);
            SRM_LOG("AFGActorRepresentationManager::SetCompassRepresentationTypeFilter: END");
        });

    SUBSCRIBE_METHOD_VIRTUAL(UFGResourceNodeRepresentation::SetupActorRepresentation,
        GetMutableDefault<UFGResourceNodeRepresentation>(),
        [](auto& scope, UFGResourceNodeRepresentation* self, AActor* realActor, bool isLocal, float lifeSpan)
        {
            SRM_LOG("UFGResourceNodeRepresentation::SetupActorRepresentation: START %s. isLocal %d. lifeSpan: %f", *realActor->GetName(), isLocal, lifeSpan);
            scope(self, realActor, isLocal, lifeSpan);
            DumpRepresentation("UFGResourceNodeRepresentation::SetupActorRepresentation: END", self);
        });

    //SUBSCRIBE_METHOD_VIRTUAL_AFTER(UFGResourceNodeRepresentation::GetRepresentationCompassMaterial,
    //    GetMutableDefault<UFGResourceNodeRepresentation>(),
    //    [](UMaterialInterface* material, const UFGResourceNodeRepresentation* self)
    //    {
    //        DumpRepresentation("UFGResourceNodeRepresentation::GetRepresentationCompassMaterial: AFTER", self);
    //    });

    SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::RemoveActorRepresentation,
        GetMutableDefault<UFGActorRepresentation>(),
        [](auto& scope, UFGActorRepresentation* self)
        {
            SRM_LOG("UFGActorRepresentation::RemoveActorRepresentation: START %s", *self->GetName());
            scope(self);
            DumpRepresentation("UFGActorRepresentation::RemoveActorRepresentation: END", self);
        });

    SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::SetupActorRepresentation,
        GetMutableDefault<UFGActorRepresentation>(),
        [](auto& scope, UFGActorRepresentation* self, AActor* realActor, bool isLocal, float lifeSpan)
        {
            SRM_LOG("UFGActorRepresentation::SetupActorRepresentation: START %s. isLocal %d. lifeSpan: %f", *realActor->GetName(), isLocal, lifeSpan);
            scope(self, realActor, isLocal, lifeSpan);
            DumpRepresentation("UFGActorRepresentation::SetupActorRepresentation: END", self);
        });

    SUBSCRIBE_METHOD_VIRTUAL(UFGActorRepresentation::TrySetupDestroyTimer,
        GetMutableDefault<UFGActorRepresentation>(),
        [](auto& scope, UFGActorRepresentation* self, float lifeSpan)
        {
            SRM_LOG("UFGActorRepresentation::TrySetupDestroyTimer: START %s. lifeSpan: %f", *self->GetName(), lifeSpan);
            scope(self, lifeSpan);
            DumpRepresentation("UFGActorRepresentation::TrySetupDestroyTimer: END", self);
        });

    SUBSCRIBE_METHOD(UFGResourceNodeRepresentation::SetupResourceNodeRepresentation,
        [](auto& scope, UFGResourceNodeRepresentation* self, class AFGResourceNodeBase* resourceNode)
        {
            DumpResourceNode("UFGResourceNodeRepresentation::SetupResourceNodeRepresentation: START", resourceNode);
            scope(self, resourceNode);
            DumpRepresentation("UFGResourceNodeRepresentation::SetupResourceNodeRepresentation: END", self);
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::ScanResourceNode_Local,
        [](auto& scope, AFGResourceNodeBase* self, float lifeSpan)
        {
            SRM_LOG("AFGResourceNodeBase::ScanResourceNode_Local: START %s. lifeSpan: %f", *self->GetName(), lifeSpan);
            DumpResourceNode("AFGResourceNodeBase::ScanResourceNode_Local", self);
            scope(self, lifeSpan);
            DumpResourceNode("AFGResourceNodeBase::ScanResourceNode_Local: END", self);
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::RemoveResourceNodeScan_Local,
        [](auto& scope, AFGResourceNodeBase* self)
        {
            DumpResourceNode("AFGResourceNodeBase::RemoveResourceNodeScan_Local: START", self);
            scope(self);
            DumpResourceNode("AFGResourceNodeBase::RemoveResourceNodeScan_Local: END", self);
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::ScanResourceNodeScan_Server,
        [](auto& scope, AFGResourceNodeBase* self)
        {
            DumpResourceNode("AFGResourceNodeBase::ScanResourceNodeScan_Server: START", self);
            scope(self);
            DumpResourceNode("AFGResourceNodeBase::ScanResourceNodeScan_Server: END", self);
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::RemoveResourceNodeScan_Server,
        [](auto& scope, AFGResourceNodeBase* self)
        {
            DumpResourceNode("AFGResourceNodeBase::RemoveResourceNodeScan_Server: START", self);
            scope(self);
            DumpResourceNode("AFGResourceNodeBase::RemoveResourceNodeScan_Server: END", self);
        });

    SUBSCRIBE_METHOD(AFGResourceNodeBase::UpdateNodeRepresentation,
        [](auto& scope, AFGResourceNodeBase* self)
        {
            DumpResourceNode("AFGResourceNodeBase::UpdateNodeRepresentation: START", self);
            scope(self);
            DumpResourceNode("AFGResourceNodeBase::UpdateNodeRepresentation: END", self);
        });

    SUBSCRIBE_METHOD_VIRTUAL(AFGResourceNodeFrackingCore::BeginPlay,
        GetMutableDefault<AFGResourceNodeFrackingCore>(),
        [&](auto& scope, AFGResourceNodeFrackingCore* self)
        {
            int32 totalSatellites = 0;
            int32 numOccupied = 0;
            numOccupied = self->GetNumOccupiedSatellites(totalSatellites);
            SRM_LOG("AFGResourceNodeFrackingCore::BeginPlay: START %s. Num Satellites: %d", *self->GetName(), totalSatellites);
            scope(self);
            numOccupied = self->GetNumOccupiedSatellites(totalSatellites);
            SRM_LOG("AFGResourceNodeFrackingCore::BeginPlay: END %s. NumSatellites: %d", *self->GetName(), totalSatellites);
        });

    SUBSCRIBE_METHOD(AFGResourceNodeFrackingCore::Factory_SetSatellitesActive,
        [&](auto& scope, AFGResourceNodeFrackingCore* self, bool isActive)
        {
            int32 totalSatellites = 0;
            int32 numOccupied = 0;
            numOccupied = self->GetNumOccupiedSatellites(totalSatellites);
            SRM_LOG("AFGResourceNodeFrackingCore::Factory_SetSatellitesActive: START %s. IsActive: %d, Num Satellites: %d", *self->GetName(), isActive, totalSatellites);
            scope(self, isActive);
            numOccupied = self->GetNumOccupiedSatellites(totalSatellites);
            SRM_LOG("AFGResourceNodeFrackingCore::Factory_SetSatellitesActive: END %s. IsActive: %d, NumSatellites: %d", *self->GetName(), isActive, totalSatellites);
        });

    SUBSCRIBE_METHOD_VIRTUAL(AFGResourceNodeFrackingSatellite::BeginPlay,
        GetMutableDefault<AFGResourceNodeFrackingSatellite>(),
        [&](auto& scope, AFGResourceNodeFrackingSatellite* self)
        {
            int32 totalSatellites = 0;
            int32 numOccupied = 0;
            numOccupied = self->GetCore()->GetNumOccupiedSatellites(totalSatellites);
            SRM_LOG("AFGResourceNodeFrackingSatellite::BeginPlay: START %s. Core ptr: %s. Num satellites: %d", *self->GetName(), *self->GetCore()->GetName(), totalSatellites);
            scope(self);

            numOccupied = self->GetCore()->GetNumOccupiedSatellites(totalSatellites);
            SRM_LOG("AFGResourceNodeFrackingSatellite::BeginPlay: END %s. Core ptr: %s. Num satellites: %d", *self->GetName(), *self->GetCore()->GetName(), totalSatellites);
        });

    SUBSCRIBE_METHOD(AFGHUD::SetCompassEntryVisibility,
        [](auto& scope, AFGHUD* self, UFGActorRepresentation* actorRepresentation, bool visible)
        {
            SRM_LOG("AFGHUD::SetCompassEntryVisibility: START %s", *self->GetName());
            scope(self, actorRepresentation, visible);
            SRM_LOG("AFGHUD::SetCompassEntryVisibility: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGHUD::OnActorRepresentationAdded,
        [](auto& scope, AFGHUD* self, UFGActorRepresentation* actorRepresentation)
        {
            SRM_LOG("AFGHUD::OnActorRepresentationAdded: START %s", *self->GetName());
            //DumpRepresentation("AFGHUD::OnActorRepresentationAdded:", actorRepresentation);
            scope(self, actorRepresentation);
            SRM_LOG("AFGHUD::OnActorRepresentationAdded: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGHUD::OnActorRepresentationRemoved,
        [](auto& scope, AFGHUD* self, UFGActorRepresentation* actorRepresentation)
        {
            SRM_LOG("AFGHUD::OnActorRepresentationRemoved: START %s", *self->GetName());
            //DumpRepresentation("AFGHUD::OnActorRepresentationRemoved:", actorRepresentation);
            scope(self, actorRepresentation);
            SRM_LOG("AFGHUD::OnActorRepresentationRemoved: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGHUD::OnActorRepresentationUpdated,
        [](auto& scope, AFGHUD* self, UFGActorRepresentation* actorRepresentation)
        {
            SRM_LOG("AFGHUD::OnActorRepresentationUpdated: START %s", *self->GetName());
            //DumpRepresentation("AFGHUD::OnActorRepresentationUpdated:", actorRepresentation);
            scope(self, actorRepresentation);
            SRM_LOG("AFGHUD::OnActorRepresentationUpdated: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGHUD::OnActorRepresentationFiltered,
        [&](auto& scope, AFGHUD* self, ERepresentationType type, bool visible)
        {
            DumpCompassEntries("AFGHUD::OnActorRepresentationFiltered START:", self->GetCompassEntries());
            scope(self, type, visible);
            DumpCompassEntries("AFGHUD::OnActorRepresentationFiltered END:", self->GetCompassEntries());
        });

    SUBSCRIBE_METHOD(AFGBuildableRadarTower::ScanForResources,
        [](auto& scope, AFGBuildableRadarTower* self)
        {
            SRM_LOG("AFGBuildableRadarTower::ScanForResources: START %s", *self->GetName());
            scope(self);
            SRM_LOG("AFGBuildableRadarTower::ScanForResources: END %s", *self->GetName());
        });

    SUBSCRIBE_METHOD(AFGBuildableRadarTower::AddResourceNodes,
        [](auto& scope, AFGBuildableRadarTower* self, const TArray< class AFGResourceNodeBase* >& resourceNodes)
        {
            SRM_LOG("AFGBuildableRadarTower::AddResourceNodes: START %s", *self->GetName());
            scope(self, resourceNodes);
            SRM_LOG("AFGBuildableRadarTower::AddResourceNodes: END %s", *self->GetName());
        });
}

void SRMDebugging::RegisterDebugHooks_Widget_MapTab(UClass* Class)
{
    if (!SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS) return;

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS
    HOOK_START_AND_RETURN(Class, SetFilterButtonNavigation);
    HOOK_START_AND_RETURN(Class, Handle Create New Marker);
    HOOK_START_AND_RETURN(Class, CleanUpHighlightedArray);
    HOOK_START_AND_RETURN(Class, OnMarkerEditorOpenChanged);
    HOOK_START_AND_RETURN(Class, Handle Edit Marker or Stamp);
    HOOK_START_AND_RETURN(Class, Remove Marker from Highlighted Array);
    HOOK_START_AND_RETURN(Class, AddMarkerToHighlightedArray);
    HOOK_START_AND_RETURN(Class, GamepadPlaceStamp);
    HOOK_START_AND_RETURN(Class, Handle Create Stamp Radial Up);
    HOOK_START_AND_RETURN(Class, Handle Create Stamp Radial);
    HOOK_START_AND_RETURN(Class, UpdateMapFilterContextMenuPosition);
    HOOK_START_AND_RETURN(Class, Handle Hide On Compass);
    HOOK_START_AND_RETURN(Class, Handle Hide On Map);
    HOOK_START_AND_RETURN(Class, HideCategoryContextMenu);
    HOOK_START_AND_RETURN(Class, HandleCategoryHighlightUntoggle);
    HOOK_START_AND_RETURN(Class, HandleCategoryHighlightToggle);
    HOOK_START_AND_RETURN(Class, Handle Access Menu Details);
    HOOK_START_AND_RETURN(Class, Handle Access Menu);
    HOOK_START_AND_RETURN(Class, SetOpenedMarker);
    HOOK_START_AND_RETURN(Class, OnNewMarkerMode);
    HOOK_START_AND_RETURN(Class, OnPlaceMarker);
    HOOK_START_AND_RETURN(Class, ExitMarkerMode);
    HOOK_START_AND_RETURN(Class, OnStampSelected);
    HOOK_START_AND_RETURN(Class, SetFiltersCollapsed);
    HOOK_START_AND_RETURN(Class, UglyFixForActorName);
    HOOK_START_AND_RETURN(Class, UpdateMapObjectVisibility);

    BEGIN_HOOK_START(Class, GetCompassRepresentation)
        LOG_LOCAL_ENUM(Type, ERepresentationType)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, GetCompassRepresentation)
        LOG_RETURN_BOOL
    FINISH_HOOK

    BEGIN_HOOK_START(Class, GetMapRepresentation)
        LOG_LOCAL_ENUM(Type, ERepresentationType)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, GetMapRepresentation)
        LOG_RETURN_BOOL
    FINISH_HOOK

    BEGIN_HOOK_START(Class, SetCompassRepresentation)
        LOG_LOCAL_ENUM(Type, ERepresentationType)
        LOG_LOCAL_BOOL(Visible)
    FINISH_HOOK
    HOOK_RETURN(Class, SetCompassRepresentation)

    BEGIN_HOOK_START(Class, SetMapRepresentation)
        LOG_LOCAL_ENUM(Type, ERepresentationType)
        LOG_LOCAL_BOOL(Visible)
    FINISH_HOOK
    HOOK_RETURN(Class, SetMapRepresentation)

    HOOK_START_AND_RETURN(Class, SetOpenMap);
}

void SRMDebugging::RegisterDebugHooks_Widget_Map(UClass* Class)
{
    if (!SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS) return;

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS
    HOOK_START_AND_RETURN(Class, RemoveMappingContext);
    HOOK_START_AND_RETURN(Class, SetMapMode);
    HOOK_START_AND_RETURN(Class, OnIconClicked);
    HOOK_START_AND_RETURN(Class, OnAttentionPingPressed);
    HOOK_START_AND_RETURN(Class, GetZOrderForType);
    HOOK_START_AND_RETURN(Class, FilterOnActors);
    HOOK_START_AND_RETURN(Class, SetHighilightViaActor);
    HOOK_START_AND_RETURN(Class, SetHighilightViaRepresentation);
    HOOK_START_AND_RETURN(Class, DiscardInput);
    HOOK_START_AND_RETURN(Class, UpdateObjectOnMap);
    BEGIN_HOOK(Class, UpdateObjectOnMap, 224)
    FINISH_HOOK
    HOOK_START_AND_RETURN(Class, OnIconUnhover);
    HOOK_START_AND_RETURN(Class, OnIconHover);

    BEGIN_HOOK_START(Class, AddObjectToMap)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, AddObjectToMap)
    FINISH_HOOK

    BEGIN_HOOK_START(Class, RemoveObjectFromMap)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, RemoveObjectFromMap)
    FINISH_HOOK

    HOOK_START_AND_RETURN(Class, FilterOnTypes);
    HOOK_START_AND_RETURN(Class, Add Mapping Context);
}

void SRMDebugging::RegisterDebugHooks_Widget_MapObject(UClass* Class)
{
    if (!SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS) return;

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS
    HOOK_START_AND_RETURN(Class, ViewRadarTowerDistance);
    HOOK_START_AND_RETURN(Class, OpenMarkerEditor);
    HOOK_START_AND_RETURN(Class, UpdateHighlight);
    HOOK_START_AND_RETURN(Class, SetMarkerEditorAnchorVisibility);
    HOOK_START_AND_RETURN(Class, OnNewColorSelected);
    HOOK_START_AND_RETURN(Class, OnNewIconSelected);
    HOOK_START_AND_RETURN(Class, CloseMarkerEditor);
    HOOK_START_AND_RETURN(Class, SetMarkerEditorClass);
    HOOK_START_AND_RETURN(Class, GetMarkerEditorContent);
    //too noisy! HOOK_START_AND_RETURN(Class, UpdateRenderScale);
    HOOK_START_AND_RETURN(Class, SetShowThisIndicator);
    HOOK_START_AND_RETURN(Class, HideViewDistanceIndicator);
    HOOK_START_AND_RETURN(Class, ShowViewDistanceIndicator);
    HOOK_START_AND_RETURN(Class, UnhighlightOnMap);
    HOOK_START_AND_RETURN(Class, HighlightOnMap);
    HOOK_START_AND_RETURN(Class, mShowActorDetails);
    HOOK_START_AND_RETURN(Class, ToggleGamepadHighlight);
}

void SRMDebugging::RegisterDebugHooks_Widget_MapCompass_Icon(UClass* Class)
{
    if (!SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS) return;

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS
    HOOK_START_AND_RETURN(Class, SetupResourceWell);

    BEGIN_HOOK_START(Class, SetupResourceIcon)
        LOG_CONTEXT_STRUCT_T(mColor, FLinearColor, DumpColor)
        LOG_CONTEXT_OBJ(mActorRepresentation)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, SetupResourceIcon)
        LOG_CONTEXT_STRUCT_T(mColor, FLinearColor, DumpColor)
        LOG_CONTEXT_OBJ(mActorRepresentation)
    FINISH_HOOK

    BEGIN_HOOK_START(Class, SetIsHighlighted)
        //LOG_CONTEXT_STRUCT_T(mColor, FLinearColor, DumpColor)
        //LOG_CONTEXT_OBJ(mActorRepresentation)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, SetIsHighlighted)
    FINISH_HOOK

    HOOK_START_AND_RETURN(Class, SetOverwriteTexture);
    HOOK_START_AND_RETURN(Class, DisplayWarning);
    HOOK_START_AND_RETURN(Class, OnTruckStationStatusChanged);
    HOOK_START_AND_RETURN(Class, OnVehicleStatusChanged);
    HOOK_START_AND_RETURN(Class, OnTrainStateChanged);
    HOOK_START_AND_RETURN(Class, SetScale);
    HOOK_START_AND_RETURN(Class, UpdateActor);
    HOOK_START_AND_RETURN(Class, SetIconType);
    HOOK_START_AND_RETURN(Class, SetDescription);
    HOOK_START_AND_RETURN(Class, SetDescription);
    HOOK_START_AND_RETURN(Class, SetRepresentatoinType);
    // Too noisy! HOOK_START_AND_RETURN(Class, SetRotation);
    HOOK_START_AND_RETURN(Class, SetIcon);
    HOOK_START_AND_RETURN(Class, UpdateMaterialType);
}

void SRMDebugging::RegisterDebugHooks_BPW_MapMenu(UClass* Class)
{
    if (!SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS) return;

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS
    HOOK_START_AND_RETURN(Class, UnfocusResources);
    HOOK_START_AND_RETURN(Class, FocusResources);

    BEGIN_HOOK_START(Class, GetGenericClass)
        LOG_LOCAL_OBJ(actorRepresentation)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, GetGenericClass)
        LOG_OUT_OBJ(Class)
        LOG_OUT_BOOL(HasGenericClass)
    FINISH_HOOK

    BEGIN_HOOK_START(Class, UnfocusGenericType)
        LOG_LOCAL_ENUM(Type, ERepresentationType)
    FINISH_HOOK
    HOOK_RETURN(Class, UnfocusGenericType);

    BEGIN_HOOK_START(Class, FocusGenericType)
        LOG_LOCAL_ENUM( Type, ERepresentationType )
    FINISH_HOOK
    HOOK_RETURN(Class, FocusGenericType);

    BEGIN_HOOK_START(Class, AddGenericActor)
        LOG_LOCAL_OBJ( Class )
        LOG_LOCAL_OBJ( actorRepresentation )
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, AddGenericActor)
        LOG_CONTEXT_MAP_T(mGenericClasses, UObject*, DumpUObjectPtr, FStruct_ActorRep, DumpFStruct_ActorRep)
    FINISH_HOOK

    BEGIN_HOOK_START(Class, UnfocusGenericClass)
        LOG_LOCAL_OBJ( Class )
        LOG_CONTEXT_MAP_T(mGenericClasses, UObject*, DumpUObjectPtr, FStruct_ActorRep, DumpFStruct_ActorRep)
        LOG_CONTEXT_MAP(mAllAddedObjects)
    FINISH_HOOK
    HOOK_RETURN(Class, UnfocusGenericClass);

    BEGIN_HOOK_START(Class, FocusGenericClass)
        LOG_LOCAL_OBJ( Class )
        LOG_CONTEXT_MAP_T(mGenericClasses, UObject*, DumpUObjectPtr, FStruct_ActorRep, DumpFStruct_ActorRep )
        LOG_CONTEXT_MAP(mAllAddedObjects)
    FINISH_HOOK
    HOOK_RETURN(Class, FocusGenericClass);

    BEGIN_HOOK_START(Class, ShouldAddToMenu)
        LOG_LOCAL_OBJ(actorRepresentation)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, ShouldAddToMenu)
        LOG_CONTEXT_MAP_T(mGenericClasses, UObject*, DumpUObjectPtr, FStruct_ActorRep, DumpFStruct_ActorRep)
        LOG_RETURN_BOOL
    FINISH_HOOK

    HOOK_START_AND_RETURN(Class, UnfocusStamps);
    HOOK_START_AND_RETURN(Class, FocusStamps);
    HOOK_START_AND_RETURN(Class, OnSearch);
    HOOK_START_AND_RETURN(Class, SetIsSearchActive);
    HOOK_START_AND_RETURN(Class, ClearData);

    BEGIN_HOOK_START(Class, RemoveActorRepresentation)
        LOG_OUT_OBJ(actorRepresentation)
    FINISH_HOOK
    HOOK_RETURN(Class, RemoveActorRepresentation)

    BEGIN_HOOK_START(Class, UpdateActorRepresentation)
        LOG_OUT_OBJ(actorRepresentation)
    FINISH_HOOK
    HOOK_RETURN(Class, UpdateActorRepresentation)

    BEGIN_HOOK_START(Class, ConvertRepresentationType)
        LOG_LOCAL_ENUM(InType, ERepresentationType)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, ConvertRepresentationType)
        LOG_OUT_ENUM(OutType, ERepresentationType)
    FINISH_HOOK

    BEGIN_HOOK_START(Class, AddCategory)
        LOG_LOCAL_ENUM(representationType, ERepresentationType)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, AddCategory)
        LOG_RETURN_OBJ
    FINISH_HOOK

    BEGIN_HOOK_START( Class, AddActorRepresentationToMenu)
        LOG_LOCAL_OBJ(actorRepresentation)
        LOG_LOCAL_OBJ(mMapObject)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, AddActorRepresentationToMenu)
        LOG_LOCAL_ENUM(LocalType, ERepresentationType)
    FINISH_HOOK

    HOOK_START_AND_RETURN(Class, Handle Show Stamps Details);
    HOOK_START_AND_RETURN(Class, Handle Show Stamps);
    HOOK_START_AND_RETURN(Class, OnActiveInputDeviceTypeChanged);
    HOOK_START_AND_RETURN(Class, GetFirstMapFilterButton);
    HOOK_START_AND_RETURN(Class, Navigate Searchbar);
    HOOK_START_AND_RETURN(Class, Handle Search);
}

void SRMDebugging::RegisterDebugHooks_BPW_MapFilterCategories(UClass* Class)
{
    if (!SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS) return;

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS
    HOOK_START_AND_RETURN(Class, DeconvertRepresentationType);
    BEGIN_HOOK_RETURN(Class, GetCategoryName)
        LOG_RETURN_TEXT
    FINISH_HOOK

    BEGIN_HOOK_START(Class, CanBeSeenOnCompass)
        LOG_LOCAL_ENUM(Index, ERepresentationType)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, CanBeSeenOnCompass)
        LOG_RETURN_BOOL
    FINISH_HOOK

    HOOK_START_AND_RETURN(Class, GetAllGenericClasses);
    HOOK_START_AND_RETURN(Class, UnfocusCategory);
    HOOK_START_AND_RETURN(Class, FocusCategory);

    BEGIN_HOOK_START(Class, SetShowOnCompass)
        LOG_LOCAL_BOOL(mShowOnCompass)
        LOG_LOCAL_BOOL(UpdateActorRepresentationManager)
    FINISH_HOOK
    HOOK_RETURN(Class, SetShowOnCompass)

    BEGIN_HOOK_START(Class, SetRepresentationType)
        LOG_LOCAL_ENUM(mRepresentationType, ERepresentationType)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, SetRepresentationType)
        LOG_LOCAL_ENUM(mRepresentationType, ERepresentationType)
    FINISH_HOOK

    BEGIN_HOOK_START(Class, SetShowOnMap)
        LOG_LOCAL_BOOL(mShowOnMap)
        LOG_LOCAL_BOOL(UpdateActorRepresentationManager)
    FINISH_HOOK
    HOOK_RETURN(Class, SetShowOnMap)

    HOOK_START_AND_RETURN(Class, GetCategoryTooltip);
    HOOK_START_AND_RETURN(Class, GetCompassButtonTooltip);
    HOOK_START_AND_RETURN(Class, GetMapButtonTooltip);
    HOOK_START_AND_RETURN(Class, SetIsSearchActive);
    HOOK_START_AND_RETURN(Class, HasAnyChildren);
    HOOK_START_AND_RETURN(Class, SetIsCollapsed);

    BEGIN_HOOK_START(Class, SetText)
        LOG_LOCAL_TEXT(mText)
    FINISH_HOOK
    HOOK_RETURN(Class, SetText)

    HOOK_START_AND_RETURN(Class, ClearChildren);
    BEGIN_HOOK_START(Class, AddChild)
        LOG_LOCAL_OBJ(Content)
    FINISH_HOOK
    HOOK_RETURN(Class, AddChild)


    HOOK_START_AND_RETURN(Class, ToggleShowOnCompass);
    HOOK_START_AND_RETURN(Class, ToggleShowOnMap);
    HOOK_START_AND_RETURN(Class, OnActiveInputChanged);
    HOOK_START_AND_RETURN(Class, CleanUpEmptySubCategories);
    HOOK_START_AND_RETURN(Class, SortSubCategories);
    HOOK_START_AND_RETURN(Class, OnMapFilterButtonUpdated);
    HOOK_START_AND_RETURN(Class, SortMapMarkerToSubCategory);
}

void SRMDebugging::RegisterDebugHooks_BPW_MapFiltersSubCategory(UClass* Class)
{
    if (!SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS) return;

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS
    HOOK_START_AND_RETURN(Class, Unfocus Subcategory);
    HOOK_START_AND_RETURN(Class, Focus Subcategory);
    HOOK_START_AND_RETURN(Class, HasAnyChilden);
    HOOK_START_AND_RETURN(Class, UpdateHeaderVisibility);
    HOOK_START_AND_RETURN(Class, SetIsSearchActive);
    HOOK_START_AND_RETURN(Class, SetIsCollapsed);
    HOOK_START_AND_RETURN(Class, GetToolTipWidget);

    BEGIN_HOOK_START(Class, SetShowOnMap)
        LOG_LOCAL_BOOL(mShowOnMap)
    FINISH_HOOK
    HOOK_RETURN(Class, SetShowOnMap)

    HOOK_START_AND_RETURN(Class, AddChild);

    BEGIN_HOOK_START(Class, SetName)
        LOG_LOCAL_TEXT(mName)
    FINISH_HOOK
    HOOK_RETURN(Class, SetName)
}

void SRMDebugging::RegisterDebugHooks_BPW_MapFilterButton(UClass* Class)
{
    if (!SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS) return;

    BEGIN_BLUEPRINT_HOOK_DEFINITIONS

    BEGIN_HOOK_START(Class, OnUnhovered)
        LOG_CONTEXT_OBJ(mGenericClass)
    FINISH_HOOK
    HOOK_RETURN(Class, OnUnhovered)

    BEGIN_HOOK_START(Class, OnHovered)
        LOG_CONTEXT_OBJ(mGenericClass)
    FINISH_HOOK
    HOOK_RETURN(Class, OnHovered)

    BEGIN_HOOK_START(Class, CustomButtonUnhovered)
        LOG_LOCAL_INT(Index)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, CustomButtonUnhovered)
    FINISH_HOOK

    BEGIN_HOOK_START(Class, CustomButtonHovered)
        LOG_LOCAL_INT(Index)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, CustomButtonHovered)
    FINISH_HOOK

    HOOK_START_AND_RETURN(Class, AddCustomButtons);

    BEGIN_HOOK_START(Class, GetName)
        LOG_CONTEXT_OBJ(mGenericClass)
        LOG_CONTEXT_OBJ(mActorRepresentation)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, GetName)
        LOG_RETURN_TEXT
    FINISH_HOOK

    BEGIN_HOOK_START(Class, SetShowOnMap)
        LOG_LOCAL_BOOL(mShowOnMap)
    FINISH_HOOK
    HOOK_RETURN(Class, SetShowOnMap);

    BEGIN_HOOK_START(Class, SetActorRepresentation)
        LOG_CONTEXT_OBJ(mGenericClass)
        LOG_LOCAL_OBJ(mActorRepresentation)
        LOG_LOCAL_BOOL(CallDelegate)
    FINISH_HOOK
    BEGIN_HOOK_RETURN(Class, SetActorRepresentation)
        LOG_CONTEXT_OBJ(mGenericClass)
    FINISH_HOOK

    HOOK_START_AND_RETURN(Class, Handle Show on Map);
}


