#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Module/GameInstanceModule.h"


class FStickyResourceMarkersModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
