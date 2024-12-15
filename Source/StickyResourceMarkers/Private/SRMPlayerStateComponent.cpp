#include "SRMPlayerStateComponent.h"

#include "FGActorRepresentation.h"
#include "FGPlayerState.h"

#include "SRMDebugging.h"
#include "SRMLogMacros.h"
#include "SRMResourceRepresentationType.h"

void USRMPlayerStateComponent::CopyProperties(USRMPlayerStateComponent* source)
{
    //SRM_LOG("USRMPlayerStateComponent::CopyProperties START. This: %p, source: %p, Started with %d, %d, %d source has %d, %d, %d",
    //    this,
    //    source,
    //    this->FilteredCompassResourceRepresentationTypes.Num(),
    //    this->FilteredMapResourceRepresentationTypes.Num(),
    //    this->CollapsedResourceRepresentationTypes.Num(),
    //    source->FilteredCompassResourceRepresentationTypes.Num(),
    //    source->FilteredMapResourceRepresentationTypes.Num(),
    //    source->CollapsedResourceRepresentationTypes.Num());

    //this->FilteredCompassResourceRepresentationTypes = TArray<uint8>(source->FilteredCompassResourceRepresentationTypes);
    //this->FilteredMapResourceRepresentationTypes = TArray<uint8>(source->FilteredMapResourceRepresentationTypes);
    //this->CollapsedResourceRepresentationTypes = TArray<uint8>(source->CollapsedResourceRepresentationTypes);

    //SRM_LOG("USRMPlayerStateComponent::CopyProperties END. This: %p, source: %p, Ended with %d, %d, %d source has %d, %d, %d",
    //    this,
    //    source,
    //    this->FilteredCompassResourceRepresentationTypes.Num(),
    //    this->FilteredMapResourceRepresentationTypes.Num(),
    //    this->CollapsedResourceRepresentationTypes.Num(),
    //    source->FilteredCompassResourceRepresentationTypes.Num(),
    //    source->FilteredMapResourceRepresentationTypes.Num(),
    //    source->CollapsedResourceRepresentationTypes.Num());
}

void USRMPlayerStateComponent::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion)
{
    SRM_LOG("USRMPlayerStateComponent::PreSaveGame_Implementation START. this: %p", this);
    //SRMDebugging::DumpEnum<ERepresentationType>("USRMPlayerStateComponent::PreSaveGame_Implementation START");

    //AFGPlayerState* ownerPlayerState = Cast<AFGPlayerState>(this->GetOwner());

    //this->FilteredCompassResourceRepresentationTypes.Empty();
    //for (auto representationType : ownerPlayerState->GetFilteredOutCompassTypes())
    //{
    //    SRM_LOG("USRMPlayerStateComponent::PreSaveGame_Implementation FilteredOutCompassType representationType %d", representationType);
    //    if (representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
    //    {
    //        this->FilteredCompassResourceRepresentationTypes.Add((uint8)representationType);
    //    }
    //}

    //this->FilteredMapResourceRepresentationTypes.Empty();
    //for (auto representationType : ownerPlayerState->GetFilteredOutMapTypes())
    //{
    //    SRM_LOG("USRMPlayerStateComponent::PreSaveGame_Implementation FilteredOutMapType representationType %d", representationType);
    //    if (representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
    //    {
    //        this->FilteredMapResourceRepresentationTypes.Add((uint8)representationType);
    //    }
    //}

    //this->CollapsedResourceRepresentationTypes.Empty();
    //for (auto representationType : ownerPlayerState->GetCollapsedMapCategories())
    //{
    //    SRM_LOG("USRMPlayerStateComponent::PreSaveGame_Implementation CollapsedMapCategory representationType %d", representationType);
    //    if (representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
    //    {
    //        this->CollapsedResourceRepresentationTypes.Add((uint8)representationType);
    //    }
    //}

    //SRMDebugging::DumpEnum<ERepresentationType>("USRMPlayerStateComponent::PreSaveGame_Implementation END");
    SRM_LOG("USRMPlayerStateComponent::PreSaveGame_Implementation END");
}

void USRMPlayerStateComponent::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion)
{
    SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation START, this: %p", this);
    //SRMDebugging::DumpEnum<ERepresentationType>("USRMPlayerStateComponent::PostLoadGame_Implementation START");
    //AFGPlayerState* ownerPlayerState = Cast<AFGPlayerState>(this->GetOwner());

    //for (auto resourceRepresentationType : this->FilteredCompassResourceRepresentationTypes)
    //{
    //    SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation:\t FilteredCompass %d", resourceRepresentationType);
    //    ownerPlayerState->Server_SetCompassFilter_Implementation((ERepresentationType)resourceRepresentationType, false);
    //}

    //for (auto resourceRepresentationType : this->FilteredMapResourceRepresentationTypes)
    //{
    //    SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation:\t FilteredMap %d", resourceRepresentationType);
    //    ownerPlayerState->Server_SetMapFilter_Implementation((ERepresentationType)resourceRepresentationType, false);
    //}

    //for (auto resourceRepresentationType : this->CollapsedResourceRepresentationTypes)
    //{
    //    SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation:\t Collapsed %d", resourceRepresentationType);
    //    ownerPlayerState->Server_SetMapCategoryCollapsed_Implementation((ERepresentationType)resourceRepresentationType, true);
    //}
    //SRMDebugging::DumpEnum<ERepresentationType>("USRMPlayerStateComponent::PostLoadGame_Implementation END");
    SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation END");
}

USRMPlayerStateComponent::USRMPlayerStateComponent()
{
    SRM_LOG("USRMPlayerStateComponent::USRMPlayerStateComponent START this: %p", this);
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;
    SRM_LOG("USRMPlayerStateComponent::USRMPlayerStateComponent END");
}

// Called when the game starts
void USRMPlayerStateComponent::BeginPlay()
{
    SRM_LOG("USRMPlayerStateComponent::BeginPlay START this: %p", this);
    Super::BeginPlay();
    SRM_LOG("USRMPlayerStateComponent::BeginPlay END");
}
