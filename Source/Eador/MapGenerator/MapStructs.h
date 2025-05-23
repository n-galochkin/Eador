﻿
#pragma once

#include "CoreMinimal.h"

#include "MapStructs.generated.h"

UENUM(BlueprintType)
enum class EHexTerrainType : uint8
{
	Water,
	Grass,
	Forest,
	Mountain
};

USTRUCT(BlueprintType)
struct EADOR_API FHexTileData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	EHexTerrainType TerrainType = EHexTerrainType::Water;

	UPROPERTY(BlueprintReadWrite)
	FIntPoint MapCoordinates;

	UPROPERTY(BlueprintReadWrite)
	FVector Position;
};

USTRUCT(BlueprintType)
struct FGridSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Width = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Height = 0;
};