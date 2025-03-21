#pragma once

#include "SRMDebugSettings.h"
#include "SRMLogCategory.h"

#if SRM_DEBUGGING_ENABLED
#define SRM_LOG(Format, ...)\
    UE_LOG(LogStickyResourceMarkers, Verbose, TEXT(Format), ##__VA_ARGS__)
#else
#define SRM_LOG(Format, ...)
#endif
