#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FGResourceNode.h"
#include "Subsystem/ModSubsystem.h"
#include "SRMNodeTrackingSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class STICKYRESOURCEMARKERS_API ASRMNodeTrackingSubsystem : public AModSubsystem, public IFGSaveInterface
{
    GENERATED_BODY()

public:
    static ASRMNodeTrackingSubsystem* Get(UWorld* World);

    // Begin IFGSaveInterface
    virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void GatherDependencies_Implementation(TArray< UObject* >& out_dependentObjects) override {};
    virtual bool NeedTransform_Implementation() override { return false; }
    virtual bool ShouldSave_Implementation() const override { return true; }
    // End IFSaveInterface

    bool NodeNeedsRepresentationRestored(AFGResourceNodeBase* node);
    bool IsNodeCurrentlyRepresented(AFGResourceNodeBase* node);
    void SetNodeRepresented(AFGResourceNodeBase* node);

    TSet<AFGResourceNodeBase*> CurrentlyRepresentedNodes;

    UPROPERTY(SaveGame)
    TSet<FName> AllRepresentedResourceNodeNames;
};
