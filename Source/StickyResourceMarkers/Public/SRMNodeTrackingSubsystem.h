#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FGResourceNode.h"
#include "Subsystem/ModSubsystem.h"
#include "SRMLogMacros.h"

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
    virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {
        SRM_LOG("ASRMNodeTrackingSubsystem::PreSaveGame_Implementation. Represented: %d, All: %d", this->AllRepresentedResourceNodeNames.Num(), this->CurrentlyRepresentedNodes.Num());
    };
    virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {
        SRM_LOG("ASRMNodeTrackingSubsystem::PostLoadGame_Implementation. Represented: %d, All: %d", this->AllRepresentedResourceNodeNames.Num(), this->CurrentlyRepresentedNodes.Num());
    };
    virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void GatherDependencies_Implementation(TArray< UObject* >& out_dependentObjects) override {};
    virtual bool NeedTransform_Implementation() override { return false; }
    virtual bool ShouldSave_Implementation() const override { return true; }
    // End IFSaveInterface

    int NumNodesNeedingRestoration() const;
    bool NodeNeedsRepresentationRestored(AFGResourceNodeBase* node) const;
    bool IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const;
    void SetNodeRepresented(AFGResourceNodeBase* node);

protected:
    UPROPERTY()
    TSet<AFGResourceNodeBase*> CurrentlyRepresentedNodes;

    UPROPERTY(SaveGame)
    TSet<FName> AllRepresentedResourceNodeNames;
};
