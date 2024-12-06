#pragma once

#include "FGActorRepresentation.h"

enum class EResourceRepresentationType : uint8
{
    // 43 is an arbitrary addition - just jumping to 50 to leave room for the game to expand
    RRT_Default = ((uint8)ERepresentationType::RT_Resource + 43),
    // Order doesn't matter here so I chose alphabetical
    RRT_Coal,
    RRT_Geyser,
    RRT_LiquidOil,
    RRT_NitrogenGas,
    RRT_OreBauxite,
    RRT_OreCopper,
    RRT_OreGold,
    RRT_OreIron,
    RRT_OreUranium,
    RRT_RawQuartz,
    RRT_SAM,
    RRT_Stone,
    RRT_Sulfur,
    RRT_Water,
};