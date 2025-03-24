#pragma once

#include "CoreMinimal.h"
#include "FGActorRepresentation.h"
#include "FGCharacterPlayer.h"
#include "FGInteractWidget.h"
#include "FGMapObjectWidget.h"
#include "FGMapWidget.h"
#include "FGResourceDescriptor.h"
#include "FGResourceNodeRepresentation.h"
#include "FGUserWidget.h"
#include "Module/GameInstanceModule.h"

#include "SRMRootInstanceModule.generated.h"

class USRMRootGameWorldModule;

UENUM(BlueprintType)
enum EResourceVisibilityLocation : uint8
{
    Compass UMETA(DisplayName = "Compass"),
    Map UMETA(DisplayName = "Map")
};

UCLASS()
class STICKYRESOURCEMARKERS_API USRMRootInstanceModule : public UGameInstanceModule
{
    GENERATED_BODY()

protected:
    void Initialize();
    void AddButtonsToMapScreen();
    UWidget* CreateResourceVisibilityButton(
        UObject* outer,
        FName name,
        FText label,
        EResourceVisibilityLocation visibilityLocation,
        bool visible,
        UWidget* templateWidget);

    void RegisterDebugHooks();

    bool TryGetResourceRepresentationType(TSubclassOf<UFGResourceDescriptor> resourceDescriptor, ERepresentationType& resourceRepresentationType);
    bool TryGetResourceRepresentationType(const AFGResourceNodeBase* resourceNode, ERepresentationType& resourceRepresentationType);
    bool TryGetResourceRepresentationType(const UFGResourceNodeRepresentation* nodeRep, ERepresentationType& resourceRepresentationType);

    bool IsResourceRepresentationType(ERepresentationType representationType) const
    {
        return representationType >= this->FirstResourceRepresentationType && representationType <= this->LastResourceRepresentationType;
    }

    // These maps live here because they only need to be populated once per game instance - they don't need to be refreshed on each load
    ERepresentationType FirstResourceRepresentationType;
    ERepresentationType LastResourceRepresentationType;

    TMap<FName, ERepresentationType> ResourceRepresentationTypeByDescriptorName;
    TMap<ERepresentationType, FText> ResourceTypeNameByResourceRepresentationType;

    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;

    // The current game world module needs to manage state that is only relevant to the currently-loaded world but this module
    // installs all the hooks before any game world module is created. The hooks that need to store world-specific state will
    // get this pointer at runtime (after the currently-loaded game world module has registered itself) to manage that state.
    static USRMRootGameWorldModule* CurrentGameWorldModule;

public:
    static USRMRootGameWorldModule* GetGameWorldModule()
    {
        return CurrentGameWorldModule;
    }

    static void SetGameWorldModule(USRMRootGameWorldModule* module)
    {
        CurrentGameWorldModule = module;
    }

    UFUNCTION(BlueprintCallable)
    void SetAllResourcesVisibility(AFGCharacterPlayer* player, EResourceVisibilityLocation location, bool visible);

    UFUNCTION(BlueprintCallable)
    void SetAllResourcesCompassVisibility(AFGCharacterPlayer* player, bool visible);

    UFUNCTION(BlueprintCallable)
    void SetAllResourcesMapVisibility(AFGCharacterPlayer* player, bool visible);

    // These are public so that the types can be set in Unreal Editor, rather than manually identifying resources by string.
    // If the types get moved, they can still fail to load but setting these in the UI is less error-prone than raw strings,
    // plus apparently there is some forwarding mechanism and if CSS does move game blueprints, those might still work.

    UPROPERTY(EditDefaultsOnly, Category = "SRM UI Types")
    TSoftClassPtr<UUserWidget> SRMResourceVisibilityButtonClass;

    // These are vanilla game UI types that need to be modified for the mod

    UPROPERTY(EditAnywhere, Category = "UI Types")
    TSoftClassPtr<UFGInteractWidget> Widget_MapContainerClass;

    UPROPERTY(EditAnywhere, Category = "UI Types")
    TSoftClassPtr<UFGMapObjectWidget> Widget_MapObjectClass;

    UPROPERTY(EditAnywhere, Category = "UI Types")
    TSoftClassPtr<UFGMapWidget> Widget_MapClass;

    UPROPERTY(EditAnywhere, Category = "UI Types")
    TSoftClassPtr<UUserWidget> BPW_MapFilterCategoriesClass;

    UPROPERTY(EditAnywhere, Category = "UI Types")
    TSoftClassPtr<UUserWidget> BPW_MapFilterButtonClass;

    UPROPERTY(EditAnywhere, Category = "UI Types")
    TSoftClassPtr<UUserWidget> Widget_MapCompass_IconClass;

    // These are only used when debugging/analyzing game behavior.

    UPROPERTY(EditAnywhere, Category = "Debug UI Types")
    TSoftClassPtr<UFGInteractWidget> Widget_MapTabClass;

    UPROPERTY(EditAnywhere, Category = "Debug UI Types")
    TSoftClassPtr<UFGUserWidget> BPW_MapMenuClass;
};

