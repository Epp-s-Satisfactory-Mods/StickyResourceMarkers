#pragma once

#include "CoreMinimal.h"
#include "FGResourceNodeBase.h"
#include "Subsystem/ModSubsystem.h"
#include "SRMClientNodeSubsystem.generated.h"

UCLASS()
class STICKYRESOURCEMARKERS_API ASRMClientNodeSubsystem : public AModSubsystem
{
    GENERATED_BODY()

public:
    static ASRMClientNodeSubsystem* Get(UWorld* World);

    ASRMClientNodeSubsystem();

    bool IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const;
    void SetNodeRepresented(AFGResourceNodeBase* node);

protected:
    UPROPERTY()
    TSet<AFGResourceNodeBase*> CurrentlyRepresentedNodes;
};
