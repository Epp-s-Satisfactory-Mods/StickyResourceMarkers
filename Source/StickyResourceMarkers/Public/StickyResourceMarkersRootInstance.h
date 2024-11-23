#pragma once

#include "CoreMinimal.h"
#include "FGActorRepresentation.h"
#include "FGInteractWidget.h"
#include "FGMapObjectWidget.h"
#include "FGMapWidget.h"
#include "FGResourceNodeRepresentation.h"
#include "FGUserWidget.h"
#include "Module/GameInstanceModule.h"

#include "StickyResourceMarkersRootInstance.generated.h"

enum class EResourceRepresentationType : uint8
{
    // 43 is an arbitrary addition - just jumping to 50 to leave room for the game to expand
    RRT_Default = ((uint8)ERepresentationType::RT_Resource + 43), 
    // Order doesn't matter here so I chose alphabetical
    RRT_Coal,
    RRT_Geyser,
    RRT_LiquidOil,
    RRT_NitrogenGas,
    RRT_OreBauxite,
    RRT_OreCopper,
    RRT_OreGold,
    RRT_OreIron,
    RRT_OreUranium,
    RRT_RawQuartz,
    RRT_SAM,
    RRT_Stone,
    RRT_Sulfur,
    RRT_Water,
};

UCLASS()
class STICKYRESOURCEMARKERS_API UStickyResourceMarkersRootInstance : public UGameInstanceModule
{
    GENERATED_BODY()

public:
    virtual void DispatchLifecycleEvent(ELifecyclePhase phase) override;

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

protected:
    void Initialize();
    void RegisterDebugHooks();
    bool TryGetResourceRepresentationType(const UFGResourceNodeRepresentation* nodeRep, EResourceRepresentationType& resourceRepresentationType);
    TMap<FString, EResourceRepresentationType> ResourceRepresentationTypeByDescriptorName;
    TMap<EResourceRepresentationType, FText> ResourceTypeNameByResourceRepresentationType;
};

