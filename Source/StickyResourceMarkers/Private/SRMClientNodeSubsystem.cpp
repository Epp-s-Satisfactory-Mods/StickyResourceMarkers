#include "SRMClientNodeSubsystem.h"

#include "Kismet/GameplayStatics.h"

#include "SRMLogMacros.h"

ASRMClientNodeSubsystem* ASRMClientNodeSubsystem::Get(UWorld* World)
{
    check(World);
    auto subsystem = Cast<ASRMClientNodeSubsystem>(UGameplayStatics::GetActorOfClass(World, StaticClass()));
    return subsystem;
}

ASRMClientNodeSubsystem::ASRMClientNodeSubsystem()
    : AModSubsystem()
{
    this->ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnClient;
}

bool ASRMClientNodeSubsystem::IsNodeCurrentlyRepresented(AFGResourceNodeBase* node) const
{
    auto isRepresented = this->CurrentlyRepresentedNodes.Contains(node);
    SRM_LOG("ASRMClientNodeSubsystem::IsNodeCurrentlyRepresented. %s, %d (there are %d represented nodes)", *node->GetName(), isRepresented, this->CurrentlyRepresentedNodes.Num());
    return isRepresented;
}

void ASRMClientNodeSubsystem::SetNodeRepresented(AFGResourceNodeBase* node)
{
    SRM_LOG("ASRMClientNodeSubsystem::SetNodeRepresented. %s", *node->GetName());
    this->CurrentlyRepresentedNodes.Add(node);
}
