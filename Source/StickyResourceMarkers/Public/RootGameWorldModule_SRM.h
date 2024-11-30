#pragma once

#include "CoreMinimal.h"
#include "FGResourceNode.h"
#include "Module/GameWorldModule.h"
#include "RootGameWorldModule_SRM.generated.h"


UCLASS()
class STICKYRESOURCEMARKERS_API URootGameWorldModule_SRM : public UGameWorldModule
{
    GENERATED_BODY()

public:
    TSet<AFGResourceNodeBase*> LateInitializedResourceNodes;
    bool AddingFromResourceScanner{ true }; // Assume it's from the resource scanner unless we know otherwise

    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;
    void RegisterLateResourceNodes();
};
