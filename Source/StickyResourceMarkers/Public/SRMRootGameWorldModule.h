#pragma once

#include "CoreMinimal.h"
#include "FGActorRepresentation.h"
#include "FGResourceNode.h"
#include "ModConfiguration.h"
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
    inline static FConfigId ConfigId{ "StickyResourceMarkers", "" };

    void Local_CreateRepresentation_Server(AFGResourceNodeBase* node);

    bool IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const;
    void SetNodeRepresented(AFGResourceNodeBase* node);

    bool IsGameInitializing{ false };
    TSet<AFGResourceNodeBase*> LateInitializedResourceNodes;

    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;

    bool GetScanningUnhidesOnCompass() const { return this->ScanningUnhidesOnCompass; }
    bool GetScanningUnhidesOnMap() const { return this->ScanningUnhidesOnMap; }
    ECompassViewDistance GetResourceCompassViewDistance() const { return this->ResourceCompassViewDistance; }

protected:
    UPROPERTY()
    ASRMClientNodeSubsystem* ClientNodeSubsystem;

    UPROPERTY()
    ASRMServerNodeSubsystem* ServerNodeSubsystem;

    UPROPERTY()
    USRMRequestRepresentNodeRCO* NodeRequestRCO;

    void Server_InitializeLateResourceNodes();
    void Server_RestoreResourceMarkers();

    void Local_SubscribeConfigUpdates();

    UFUNCTION()
    void UpdateConfig();

    void UpdateConfigValues();
    void UpdateHUDRepresentations();

    bool ScanningUnhidesOnCompass{ true };
    bool ScanningUnhidesOnMap{ false };
    ECompassViewDistance ResourceCompassViewDistance{ ECompassViewDistance::CVD_Always };
};
