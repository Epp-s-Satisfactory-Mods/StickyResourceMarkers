#include "SRMNodeTrackingSubsystem.h"


#include "Kismet/GameplayStatics.h"

#include "SRMLogMacros.h"

ASRMNodeTrackingSubsystem* ASRMNodeTrackingSubsystem::Get(UWorld* World)
{
    check(World);
    return Cast<ASRMNodeTrackingSubsystem>(UGameplayStatics::GetActorOfClass(World, StaticClass()));
}

bool ASRMNodeTrackingSubsystem::NodeNeedsRepresentationRestored(AFGResourceNodeBase* node)
{
    if (IsNodeCurrentlyRepresented(node))
    {
        return false;
    }

    return this->AllRepresentedResourceNodeNames.Contains(node->GetFName());
}

bool ASRMNodeTrackingSubsystem::IsNodeCurrentlyRepresented(AFGResourceNodeBase* node)
{
    return this->CurrentlyRepresentedNodes.Contains(node);
}

void ASRMNodeTrackingSubsystem::SetNodeRepresented(AFGResourceNodeBase* node)
{
    this->CurrentlyRepresentedNodes.Add(node);
    this->AllRepresentedResourceNodeNames.Add(node->GetFName());
}
