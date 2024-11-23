#pragma once

#include "CoreMinimal.h"

class SRMDebugging
{
public:
	static void RegisterNativeDebugHooks();
	static void RegisterDebugHooks_Widget_MapTab(UClass* Class);
	static void RegisterDebugHooks_Widget_Map(UClass* Class);
	static void RegisterDebugHooks_Widget_MapObject(UClass* Class);
	static void RegisterDebugHooks_Widget_MapCompass_Icon(UClass* Class);
	static void RegisterDebugHooks_BPW_MapMenu(UClass* Class);
	static void RegisterDebugHooks_BPW_MapFilterCategories(UClass* Class);
	static void RegisterDebugHooks_BPW_MapFiltersSubCategory(UClass* Class);
	static void RegisterDebugHooks_BPW_MapFilterButton(UClass* Class);
};
