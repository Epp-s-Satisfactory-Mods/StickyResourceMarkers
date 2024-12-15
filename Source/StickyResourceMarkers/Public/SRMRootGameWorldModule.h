#pragma once

#include "CoreMinimal.h"
#include "FGResourceNode.h"
#include "Module/GameWorldModule.h"

#include "SRMClientNodeSubsystem.h"
#include "SRMRequestRepresentNodeRCO.h"
#include "SRMServerNodeSubsystem.h"

#include "SRMRootGameWorldModule.generated.h"

UCLASS()
class STICKYRESOURCEMARKERS_API USRMRootGameWorldModule : public UGameWorldModule
{
    GENERATED_BODY()

public:
    void Local_CreateRepresentation_Server(AFGResourceNodeBase* node);

    bool IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const;
    void SetNodeRepresented(AFGResourceNodeBase* node);

    bool IsGameInitializing{ false };
    TSet<AFGResourceNodeBase*> LateInitializedResourceNodes;

    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;

protected:
    UPROPERTY()
    ASRMClientNodeSubsystem* ClientNodeSubsystem;

    UPROPERTY()
    ASRMServerNodeSubsystem* ServerNodeSubsystem;

    UPROPERTY()
    USRMRequestRepresentNodeRCO* NodeRequestRCO;

    void Server_InitializeLateResourceNodes();
    void Server_RestoreResourceMarkers();
};
