#pragma once

#include "CoreMinimal.h"
#include "FGSaveInterface.h"
#include "FGResourceNode.h"
#include "Subsystem/ModSubsystem.h"
#include "SRMLogMacros.h"

#include "SRMServerNodeSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class STICKYRESOURCEMARKERS_API ASRMServerNodeSubsystem : public AModSubsystem, public IFGSaveInterface
{
    GENERATED_BODY()

public:
    static ASRMServerNodeSubsystem* Get(UWorld* World);

    // Begin IFGSaveInterface
    virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
    virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override {};
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
