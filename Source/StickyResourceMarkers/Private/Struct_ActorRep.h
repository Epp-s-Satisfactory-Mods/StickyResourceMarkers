#pragma once

#include "CoreMinimal.h"
#include "FGActorRepresentation.h"

#include "Struct_ActorRep.generated.h"

USTRUCT(BlueprintType)
struct FStruct_ActorRep
{
	GENERATED_BODY()
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayName = "ActorRepresentations"))
	TArray<UFGActorRepresentation*> ActorRepresentations;
};
