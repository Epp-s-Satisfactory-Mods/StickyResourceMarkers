#include "SRMPlayerStateComponent.h"

#include "FGActorRepresentation.h"
#include "FGPlayerState.h"

#include "SRMLogMacros.h"
#include "SRMResourceRepresentationType.h"

void USRMPlayerStateComponent::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion)
{
    SRM_LOG("USRMPlayerStateComponent::PreSaveGame_Implementation START");
    AFGPlayerState* ownerPlayerState = Cast<AFGPlayerState>(this->GetOwner());

    this->FilteredCompassResourceRepresentationTypes.Empty();
    for (auto representationType : ownerPlayerState->GetFilteredOutCompassTypes())
    {
        if (representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            this->FilteredCompassResourceRepresentationTypes.Add((uint8)representationType);
        }
    }

    this->FilteredMapResourceRepresentationTypes.Empty();
    for (auto representationType : ownerPlayerState->GetFilteredOutMapTypes())
    {
        if (representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            this->FilteredMapResourceRepresentationTypes.Add((uint8)representationType);
        }
    }

    this->CollapsedResourceRepresentationTypes.Empty();
    for (auto representationType : ownerPlayerState->GetCollapsedMapCategories())
    {
        if (representationType > (ERepresentationType)ESRMResourceRepresentationType::RRT_Default)
        {
            this->CollapsedResourceRepresentationTypes.Add((uint8)representationType);
        }
    }

    SRM_LOG("USRMPlayerStateComponent::PreSaveGame_Implementation END");
}

void USRMPlayerStateComponent::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion)
{
    SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation START");
    AFGPlayerState* ownerPlayerState = Cast<AFGPlayerState>(this->GetOwner());

    for (auto resourceRepresentationType : this->FilteredCompassResourceRepresentationTypes)
    {
        SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation:\t FilteredCompass %d", resourceRepresentationType);
        ownerPlayerState->Server_SetCompassFilter_Implementation((ERepresentationType)resourceRepresentationType, false);
    }

    for (auto resourceRepresentationType : this->FilteredMapResourceRepresentationTypes)
    {
        SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation:\t FilteredMap %d", resourceRepresentationType);
        ownerPlayerState->Server_SetMapFilter_Implementation((ERepresentationType)resourceRepresentationType, false);
    }

    for (auto resourceRepresentationType : this->CollapsedResourceRepresentationTypes)
    {
        SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation:\t Collapsed %d", resourceRepresentationType);
        ownerPlayerState->Server_SetMapCategoryCollapsed_Implementation((ERepresentationType)resourceRepresentationType, true);
    }

    SRM_LOG("USRMPlayerStateComponent::PostLoadGame_Implementation END");
}

USRMPlayerStateComponent::USRMPlayerStateComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void USRMPlayerStateComponent::BeginPlay()
{
    Super::BeginPlay();
}
