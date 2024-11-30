#include "StickyResourceMarkers.h"
#include "SRMLogMacros.h"

// The mod template does this but we have no text to localize
#define LOCTEXT_NAMESPACE "FStickyResourceMarkersModule"

void FStickyResourceMarkersModule::StartupModule()
{
    SRM_LOG("STARTUP MODULE");
}

void FStickyResourceMarkersModule::ShutdownModule()
{
    SRM_LOG("SHUTDOWN MODULE");
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStickyResourceMarkersModule, StickyResourceMarkers)