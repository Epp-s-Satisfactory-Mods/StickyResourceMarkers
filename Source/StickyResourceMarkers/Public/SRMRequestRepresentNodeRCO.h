

#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "FGResourceNodeBase.h"
#include "SRMRequestRepresentNodeRCO.generated.h"

/**
 * 
 */
UCLASS()
class STICKYRESOURCEMARKERS_API USRMRequestRepresentNodeRCO : public UFGRemoteCallObject
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(Server, Reliable)
    void CreateRepresentation_Server(AFGResourceNodeBase* node);

    UPROPERTY(Replicated)
    bool bDummyToForceReplication = true;
};
