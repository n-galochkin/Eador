#include "MapGenerator.h"

#include "HexActor.h"
#include "MapStructs.h"
#include "Engine/AssetManager.h"
#include "Math/UnrealMathUtility.h"

#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(EAMapGenerator, Log, All);
#define CHANNEL(Verbosity, Format, ...) UE_LOGFMT(EAMapGenerator, Verbosity, Format, ##__VA_ARGS__)
#define LOG(Format, ...) CHANNEL(Log, Format, ##__VA_ARGS__)
#define WARNING(Format, ...) CHANNEL(Warning, Format, ##__VA_ARGS__)
#define ERROR(Format, ...) CHANNEL(Error, Format, ##__VA_ARGS__)

UMapGenerator::UMapGenerator()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMapGenerator::BeginPlay()
{
	Super::BeginPlay();

	InitializeHexActorPool(MapSize.Width * MapSize.Height);

	if (MapAssets)
	{
		UAssetManager::GetIfInitialized()->LoadPrimaryAsset(MapAssets->GetPrimaryAssetId(), {"Terrain"},
															FStreamableDelegate::CreateWeakLambda(this, [this]()
															{
																RegenerateMap();
															}));
	}
}

void UMapGenerator::RegenerateMap()
{
	LOG("Regenerating map...");

	GenerateLogicalMap();
	SpawnHexActors();
}

void UMapGenerator::GenerateLogicalMap()
{
	LogicalMap.Empty(); // Очищаем предыдущую карту, если она была

	const int32 Seed = FMath::Rand();
	float MinNoise = 1.0f;
	float MaxNoise = 0.0f;
	const float HexHeight = HexSize * FMath::Sqrt(3.f); // Высота гекса

	for (int32 Q = 0; Q < MapSize.Width; ++Q)
	{
		for (int32 R = 0; R < MapSize.Height; ++R)
		{
			float NoiseX = (Q + Seed) * NoiseScale;
			float NoiseY = (R + Seed) * NoiseScale;
			const float NoiseValue = FMath::PerlinNoise2D(FVector2D(NoiseX, NoiseY));
			if (NoiseValue < MinNoise)
			{
				MinNoise = NoiseValue;
			}
			if (NoiseValue > MaxNoise)
			{
				MaxNoise = NoiseValue;
			}
			
			const float CenterX = Q * HexSize * 1.5f;
			const float CenterZ = R * HexHeight + (Q % 2 == 1 ? HexHeight * 0.5f : 0.0f);
			FVector Center(CenterX, 0.0f, CenterZ);

			// Сохраняем данные о гексе
			FHexTileData HexData;
			HexData.MapCoordinates = FIntPoint(Q, R);
			HexData.Position = Center;
			HexData.TerrainType = NoiseValue < TerrainThresholds[EHexTerrainType::Grass] ? EHexTerrainType::Water : NoiseValue < TerrainThresholds[EHexTerrainType::Forest] ? EHexTerrainType::Grass : NoiseValue < TerrainThresholds[EHexTerrainType::Mountain] ? EHexTerrainType::Forest : EHexTerrainType::Mountain;;
			LogicalMap.Add(HexData);
		}
	}

	LOG("Generated Hex with MinNoise {0} MaxNoise {1}", MinNoise, MaxNoise);
}

void UMapGenerator::InitializeHexActorPool(int32 InitialPoolSize)
{
	for (int32 i = 0; i < InitialPoolSize; ++i)
	{
		AddNewHexActorToPool();
	}
}

void UMapGenerator::AddNewHexActorToPool()
{
	if (AHexActor* HexActor = GetWorld()->SpawnActor<AHexActor>(FVector::ZeroVector, FRotator::ZeroRotator))
	{
		ReturnHexActorToPool(HexActor);
	}
}

AHexActor* UMapGenerator::GetHexActorFromPool()
{
	if (HexActorPool.IsEmpty())
	{
		AddNewHexActorToPool();
		LOG("Spawned new HexActor to the pool (Actors count: {0})", HexActorPool.Num() + ActiveHexActors.Num());
	}

	AHexActor* HexActor = HexActorPool.Pop();
	HexActor->SetActorHiddenInGame(false);
	return HexActor;
}

void UMapGenerator::ReturnHexActorToPool(AHexActor* HexActor)
{
	HexActor->SetActorHiddenInGame(true);
	HexActorPool.Add(HexActor);
}

void UMapGenerator::SpawnHexActors()
{
	for (AHexActor* HexActor : ActiveHexActors)
	{
		ReturnHexActorToPool(HexActor);
	}
	ActiveHexActors.Empty();

	// Карта для быстрого поиска HexActor по координатам (Q, R)
	TMap<FIntPoint, AHexActor*> HexActorMap;

	for (auto HexData : LogicalMap)
	{
		if (AHexActor* HexActor = GetHexActorFromPool())
		{
			HexActor->SetHexData(HexData);
			HexActor->SetMaterial(MapAssets->TerrainMaterials[HexData.TerrainType].Get());
			ActiveHexActors.Add(HexActor);

			HexActorMap.Add(HexData.MapCoordinates, HexActor);
		}
	}

	for (AHexActor* HexActor : ActiveHexActors)
	{
		HexActor->FillHexNeighbors(HexActorMap);
		HexActor->GenerateMesh(HexSize);
	}
}
