#include "SRMNodeTrackingSubsystem.h"

#include "Kismet/GameplayStatics.h"

#include "SRMLogMacros.h"

ASRMNodeTrackingSubsystem* ASRMNodeTrackingSubsystem::Get(UWorld* World)
{
    check(World);
    auto subsystem = Cast<ASRMNodeTrackingSubsystem>(UGameplayStatics::GetActorOfClass(World, StaticClass()));
    return subsystem;
}

int ASRMNodeTrackingSubsystem::NumNodesNeedingRestoration() const
{
    // Something cannot get into CurrentlyRepresented without also being added to AllRepresented
    return this->AllRepresentedResourceNodeNames.Num() - this->CurrentlyRepresentedNodes.Num();
}

bool ASRMNodeTrackingSubsystem::NodeNeedsRepresentationRestored(AFGResourceNodeBase* node) const
{
    if (IsNodeCurrentlyRepresented(node))
    {
        return false;
    }

    return this->AllRepresentedResourceNodeNames.Contains(node->GetFName());
}

bool ASRMNodeTrackingSubsystem::IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const
{
    return this->CurrentlyRepresentedNodes.Contains(node);
}

void ASRMNodeTrackingSubsystem::SetNodeRepresented(AFGResourceNodeBase* node)
{
    SRM_LOG("ASRMNodeTrackingSubsystem::SetNodeRepresented. %s", *node->GetName());
    this->CurrentlyRepresentedNodes.Add(node);
    this->AllRepresentedResourceNodeNames.Add(node->GetFName());
}
