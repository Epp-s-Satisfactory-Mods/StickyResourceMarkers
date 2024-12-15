#pragma once

#include "CoreMinimal.h"
#include "FGResourceNode.h"
#include "Module/GameWorldModule.h"
#include "SRMClientNodeSubsystem.h"
#include "SRMNodeTrackingSubsystem.h"
#include "SRMResourceRepresentationType.h"
#include "SRMRequestRepresentNodeRCO.h"

#include "RootGameWorldModule_SRM.generated.h"

UCLASS()
class STICKYRESOURCEMARKERS_API URootGameWorldModule_SRM : public UGameWorldModule
{
    GENERATED_BODY()

public:
    void Local_CreateRepresentation_Server(AFGResourceNodeBase* node);

    bool IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const;
    void SetNodeRepresented(AFGResourceNodeBase* node);

    bool IsGameInitializing{ false };
    TSet<AFGResourceNodeBase*> LateInitializedResourceNodes;
    bool AddingFromOtherThanResourceScanner{ false }; // Assume it's from the resource scanner unless we know otherwise

    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;

protected:
    UPROPERTY()
    ASRMClientNodeSubsystem* ClientNodeSubsystem;

    UPROPERTY()
    ASRMNodeTrackingSubsystem* NodeTrackingSubsystem;

    UPROPERTY()
    USRMRequestRepresentNodeRCO* NodeRequestRCO;

    void Server_InitializeLateResourceNodes();
    void Server_RestoreResourceMarkers();
};
