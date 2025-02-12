#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "SRMConfigurationStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/StickyResourceMarkers/SRMConfiguration' */
USTRUCT(BlueprintType)
struct FSRMConfigurationStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    bool ScanningUnhidesOnCompass{};

    UPROPERTY(BlueprintReadWrite)
    bool ScanningUnhidesOnMap{};

    UPROPERTY(BlueprintReadWrite)
    int32 ResourceCompassViewDistance{};

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FSRMConfigurationStruct GetActiveConfig(UObject* WorldContext) {
        FSRMConfigurationStruct ConfigStruct{};
        FConfigId ConfigId{"StickyResourceMarkers", ""};
        if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull)) {
            UConfigManager* ConfigManager = World->GetGameInstance()->GetSubsystem<UConfigManager>();
            ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FSRMConfigurationStruct::StaticStruct(), &ConfigStruct});
        }
        return ConfigStruct;
    }
};

