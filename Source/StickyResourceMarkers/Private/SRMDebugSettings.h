#pragma once

// When we're building to ship, set this to 0 to no-op ALL logging and debugging functions to minimize performance impact. Would prefer to do
// this through build defines based on whether we're building for development or shipping but at the moment alpakit always builds shipping.
#define SRM_DEBUGGING_ENABLED 1

// If debugging is enabled, whether to also register hooks for mod functionality. Useful for disabling the mod to trace default game behavior.
#define SRM_DEBUGGING_REGISTER_MOD_HOOKS 1

// If debugging is enabled, this must be 1 to trace calls to blueprint functions that are not necessary for the mod but helpful for understanding.
#define SRM_DEBUGGING_TRACE_ALL_BLUEPRINT_HOOKS 0

// If debugging is enabled, this must be 1 to trace calls to native functions that are not necessary for the mod but helpful for understanding.
#define SRM_DEBUGGING_TRACE_ALL_NATIVE_HOOKS 1
