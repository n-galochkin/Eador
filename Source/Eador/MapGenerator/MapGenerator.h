// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapAssets.h"
#include "MapStructs.h"
#include "Components/ActorComponent.h"
#include "MapGenerator.generated.h"


UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EADOR_API UMapGenerator : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMapGenerator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMapAssets* MapAssets = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGridSize MapSize = FGridSize(10, 10);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HexSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NoiseScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EHexTerrainType, float> TerrainThresholds = {
		{EHexTerrainType::Water, 0.0f},
		{EHexTerrainType::Grass, 0.2f},
		{EHexTerrainType::Forest, 0.4f},
		{EHexTerrainType::Mountain, 0.6f}
	};

	TArray<FHexTileData> LogicalMap;

	UPROPERTY()
	TArray<class AHexActor*> HexActorPool;

	UPROPERTY()
	TArray<class AHexActor*> ActiveHexActors;

	void RegenerateMap();

	void GenerateLogicalMap();


	void InitializeHexActorPool(int32 InitialPoolSize);
	void AddNewHexActorToPool();
	AHexActor* GetHexActorFromPool();
	void ReturnHexActorToPool(AHexActor* HexActor);

	void SpawnHexActors();
};
