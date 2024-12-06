#pragma once

#include "CoreMinimal.h"
#include "FGActorRepresentation.h"
#include "FGInteractWidget.h"
#include "FGMapObjectWidget.h"
#include "FGMapWidget.h"
#include "FGResourceNodeRepresentation.h"
#include "FGUserWidget.h"
#include "Module/GameInstanceModule.h"
#include "SRMResourceRepresentationType.h"

#include "StickyResourceMarkersRootInstance.generated.h"

class URootGameWorldModule_SRM;

UCLASS()
class STICKYRESOURCEMARKERS_API UStickyResourceMarkersRootInstance : public UGameInstanceModule
{
    GENERATED_BODY()

protected:
    void Initialize();
    void RegisterDebugHooks();
    bool TryGetResourceRepresentationType(const AFGResourceNodeBase* resourceNode, EResourceRepresentationType& resourceRepresentationType);
    bool TryGetResourceRepresentationType(const UFGResourceNodeRepresentation* nodeRep, EResourceRepresentationType& resourceRepresentationType);

    // These maps live here because they only need to be populated once per game instance - they don't need to be refreshed on each load
    TMap<FString, EResourceRepresentationType> ResourceRepresentationTypeByDescriptorName;
    TMap<EResourceRepresentationType, FText> ResourceTypeNameByResourceRepresentationType;

    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;

    // The current game world module needs to manage state that is only relevant to the currently-loaded world but this module
    // installs all the hooks before any game world module is created. The hooks that need to store world-specific state will
    // get this pointer at runtime (after the currently-loaded game world module has registered itself) to manage that state.
    static URootGameWorldModule_SRM* CurrentGameWorldModule;

public:
    static URootGameWorldModule_SRM* GetGameWorldModule()
    {
        return CurrentGameWorldModule;
    }

    static void SetGameWorldModule(URootGameWorldModule_SRM* module)
    {
        CurrentGameWorldModule = module;
    }

    // These are public so that the types can be set in Unreal Editor, rather than manually identifying resources by string.
    // If the types get moved, they can still fail to load but setting (and fixing) these in the UI is less error-prone than
    // raw strings, plus apparently there is some forwarding mechanism and if CSS does move them, these might still work.
    // Not all of these are needed for the mod, but many are related and are/were helpful in analyzing and debugging

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UFGInteractWidget> Widget_MapContainerClass;

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UFGInteractWidget> Widget_MapTabClass;

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UFGMapWidget> Widget_MapClass;

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UFGMapObjectWidget> Widget_MapObjectClass;

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UUserWidget> Widget_MapCompass_IconClass;

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UFGUserWidget> BPW_MapMenuClass;

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UUserWidget> BPW_MapFilterCategoriesClass;

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UUserWidget> BPW_MapFiltersSubCategoryClass;

    UPROPERTY(EditAnywhere, Category = "UI Widget Types")
    TSoftClassPtr<UUserWidget> BPW_MapFilterButtonClass;
};

