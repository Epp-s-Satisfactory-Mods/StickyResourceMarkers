#include "SRMServerNodeSubsystem.h"

#include "Kismet/GameplayStatics.h"

#include "SRMLogMacros.h"

ASRMServerNodeSubsystem* ASRMServerNodeSubsystem::Get(UWorld* World)
{
    check(World);
    auto subsystem = Cast<ASRMServerNodeSubsystem>(UGameplayStatics::GetActorOfClass(World, StaticClass()));
    return subsystem;
}

int ASRMServerNodeSubsystem::NumNodesNeedingRestoration() const
{
    // Something cannot get into CurrentlyRepresented without also being added to AllRepresented
    return this->AllRepresentedResourceNodeNames.Num() - this->CurrentlyRepresentedNodes.Num();
}

bool ASRMServerNodeSubsystem::NodeNeedsRepresentationRestored(AFGResourceNodeBase* node) const
{
    if (IsNodeCurrentlyRepresented(node))
    {
        return false;
    }

    return this->AllRepresentedResourceNodeNames.Contains(node->GetFName());
}

bool ASRMServerNodeSubsystem::IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const
{
    return this->CurrentlyRepresentedNodes.Contains(node);
}

void ASRMServerNodeSubsystem::SetNodeRepresented(AFGResourceNodeBase* node)
{
    SRM_LOG("ASRMServerNodeSubsystem::SetNodeRepresented. %s", *node->GetName());
    this->CurrentlyRepresentedNodes.Add(node);
    this->AllRepresentedResourceNodeNames.Add(node->GetFName());
}
