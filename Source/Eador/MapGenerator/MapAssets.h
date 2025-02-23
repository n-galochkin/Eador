// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapStructs.h"
#include "Engine/DataAsset.h"
#include "MapAssets.generated.h"


UCLASS(Blueprintable)
class EADOR_API UMapAssets : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AssetBundles = "Terrain"))
	TMap<EHexTerrainType, TSoftObjectPtr<UMaterialInstance>> TerrainMaterials;
};
