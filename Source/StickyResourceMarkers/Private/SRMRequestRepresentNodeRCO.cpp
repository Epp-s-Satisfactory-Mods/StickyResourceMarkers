#include "SRMRequestRepresentNodeRCO.h"

#include "Net/UnrealNetwork.h"
#include "SRMDebugging.h"
#include "SRMLogMacros.h"

void USRMRequestRepresentNodeRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(USRMRequestRepresentNodeRCO, bDummyToForceReplication);
}

void USRMRequestRepresentNodeRCO::CreateRepresentation_Server_Implementation(AFGResourceNodeBase* node)
{
    SRM_LOG("USRMRequestRepresentNodeRCO::CreateRepresentation_Server: START");
    check(node);
    SRMDebugging::DumpResourceNode(TEXT("USRMRequestRepresentNodeRCO::CreateRepresentation_Server"), node);
    node->ScanResourceNodeScan_Server();
    SRMDebugging::DumpResourceNode(TEXT("USRMRequestRepresentNodeRCO::CreateRepresentation_Server"), node);
    SRM_LOG("USRMRequestRepresentNodeRCO::CreateRepresentation_Server: END");
}
