#pragma once

#include "CoreMinimal.h"

#include "FGActorRepresentation.h"
#include "FGResourceNode.h"
#include "FGResourceNodeRepresentation.h"
#include "FGHUD.h"
#include "GameFramework/PlayerState.h"
#include "Struct_ActorRep.h"
#include "Widget.h"

#include "SRMLogMacros.h"

class SRMDebugging
{
public:
	static void RegisterNativeDebugHooks();
	static void RegisterDebugHooks_Widget_MapContainer(UClass* Class);
	static void RegisterDebugHooks_Widget_MapTab(UClass* Class);
	static void RegisterDebugHooks_Widget_Map(UClass* Class);
	static void RegisterDebugHooks_Widget_MapObject(UClass* Class);
	static void RegisterDebugHooks_Widget_MapCompass_Icon(UClass* Class);
	static void RegisterDebugHooks_BPW_MapMenu(UClass* Class);
	static void RegisterDebugHooks_BPW_MapFilterCategories(UClass* Class);
	static void RegisterDebugHooks_BPW_MapFiltersSubCategory(UClass* Class);
	static void RegisterDebugHooks_BPW_MapFilterButton(UClass* Class);

	static void DumpWidget(FString prefix, const UWidget* widget);
	static void DumpResourceNode(FString prefix, const AFGResourceNodeBase* res, bool shortDump = true);
	static void DumpRepresentation(FString prefix, const UFGActorRepresentation* rep, bool shortDump = true);

	template<typename TKey, typename TValue>
	static void DumpFScriptMap(FString prefix, FScriptMapHelper& mapHelper, TFunction<void(FString,TKey*)> keyDumper, TFunction<void(FString, TValue*)> valueDumper)
	{
		SRM_LOG("%s FScriptMap:", *prefix);
		auto nestedPrefix = prefix + "\t";
		SRM_LOG("%s ASDF Count: %d", *nestedPrefix, mapHelper.Num());
		nestedPrefix += "\t";
		for (auto iter = mapHelper.CreateIterator(); iter; ++iter)
		{
			auto index = *iter;
			auto keyPtr = (TKey*)mapHelper.GetKeyPtr(index);
			auto valuePtr = (TValue*)mapHelper.GetValuePtr(index);

			keyDumper(nestedPrefix + " Key:", keyPtr);
			valueDumper(nestedPrefix + " Value:", valuePtr);
		}
	}

	static void DumpFStruct_ActorRep(FString prefix, FStruct_ActorRep* actorRep);

	template<typename TEnum>
	static void DumpEnum(FString prefix, TEnum* testValue = nullptr)
	{
		auto e = StaticEnum<TEnum>();

		SRM_LOG("%s %s", *prefix, *e->GetName());
		auto nestedPrefix = prefix + "\t";
		for (int i = 0; i < 256; ++i)
		{
			if (e->IsValidEnumValue(i))
			{
				SRM_LOG("%s %d: %s", *nestedPrefix, i, *e->GetNameStringByValue(i))
			}
		}

		if (testValue)
		{
			SRM_LOG("%s Test Value %d IsValidEnumValue: %d", *nestedPrefix, *testValue, e->IsValidEnumValue((int64)*testValue));
		}
	}

	static void DumpColor(FString prefix, FLinearColor color);
	static void DumpUObjectPtr(FString prefix, UObject** object);
	static void DumpUObject(FString prefix, UObject* object);
	static void DumpPlayerState(FString prefix, APlayerState* state);
	static void DumpCompassEntry(FString prefix, FCompassEntry& compassEntry, int* indexPtr = nullptr, bool shortDump = true);
	static void DumpMatchingCompassEntries(FString prefix, TArray<FCompassEntry>& compassEntries, UFGResourceNodeRepresentation* rep, bool shortDump = true);
	static void DumpCompassEntries(FString prefix, TArray<FCompassEntry>& compassEntries, bool shortDump = true);
};
