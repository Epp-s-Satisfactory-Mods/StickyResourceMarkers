#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FGSaveInterface.h"

#include "SRMPlayerStateComponent.generated.h"

UCLASS(Blueprintable)
class STICKYRESOURCEMARKERS_API USRMPlayerStateComponent : public UActorComponent, public IFGSaveInterface
{
    GENERATED_BODY()

public:
    void CopyProperties(USRMPlayerStateComponent* source);

    // Begin IFGSaveInterface
    virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
    virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;

    virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void GatherDependencies_Implementation(TArray< UObject* >& out_dependentObjects) override {};
    virtual bool NeedTransform_Implementation() override { return false; }
    virtual bool ShouldSave_Implementation() const override { return true; }
    // End IFSaveInterface

    USRMPlayerStateComponent();

    virtual void BeginPlay() override;

    UPROPERTY(SaveGame)
    TArray<uint8> FilteredCompassResourceRepresentationTypes;
    UPROPERTY(SaveGame)
    TArray<uint8> FilteredMapResourceRepresentationTypes;
    UPROPERTY(SaveGame)
    TArray<uint8> CollapsedResourceRepresentationTypes;
};
