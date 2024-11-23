#include "StickyResourceMarkers.h"

// The mod template does this but we have no text to localize
#define LOCTEXT_NAMESPACE "FStickyResourceMarkersModule"

void FStickyResourceMarkersModule::StartupModule()
{
}

void FStickyResourceMarkersModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FStickyResourceMarkersModule, StickyResourceMarkers)