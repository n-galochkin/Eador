#include "MapGenerator.h"

#include "HexActor.h"
#include "MapStructs.h"
#include "ProceduralMeshActor.h"
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

#if WITH_EDITOR
void UMapGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		RegenerateMap();
	}
}
#endif

void UMapGenerator::RegenerateMap()
{
	LOG("Regenerating map...");

	GenerateLogicalMap();
	SpawnHexActors();
}

void UMapGenerator::GenerateLogicalMap()
{
	LogicalMap.Empty(); // Очищаем предыдущую карту, если она была

	const float HexHeight = HexSize * FMath::Sqrt(3.f); // Высота гекса

	const int32 NumQ = static_cast<float>(MapSize.Width) * 2.f;
	const int32 NumR = static_cast<float>(MapSize.Height) * 0.5f;

	for (int32 Q = 0; Q < NumQ; ++Q)
	{
		for (int32 R = 0; R < NumR; ++R)
		{
			// Вычисляем значение Perlin Noise для этого гекса
			const float NoiseValue = FMath::PerlinNoise2D(FVector2D(Q * NoiseScale, R * NoiseScale));

			const float CenterX = Q * HexSize * 0.5f;
			float CenterZ = R * HexHeight;
			if (Q % 2 == 1) CenterZ += HexHeight * 0.5f; // Смещение для нечетных колонок
			FVector Center(CenterX, 0.0f, CenterZ);

			// Углы для flat-top гекса
			TArray<FVector> HexVertices;
			for (int32 i = 0; i < 6; ++i)
			{
				float AngleDeg = 60.0f * i - 30.0f; 
				float AngleRad = FMath::DegreesToRadians(AngleDeg);
				 HexVertices.Add(FVector(
					CenterX + FMath::RoundToInt(HexSize * FMath::Cos(AngleRad) * 1000.0f) / 1000.0f,
					0.0f,
					CenterZ + FMath::RoundToInt(HexSize * FMath::Sin(AngleRad) * 1000.0f) / 1000.0f
				));
			}
			HexVertices.Add(Center); // Центральная вершина

			// Сохраняем данные о гексе
			FHexTileData HexData;
			HexData.Q = Q;
			HexData.R = R;
			HexData.Position = Center;
			HexData.Vertices = HexVertices;
			HexData.TerrainType = NoiseValue < TerrainThresholds[EHexTerrainType::Grass] ? EHexTerrainType::Water : NoiseValue < TerrainThresholds[EHexTerrainType::Forest] ? EHexTerrainType::Grass : NoiseValue < TerrainThresholds[EHexTerrainType::Mountain] ? EHexTerrainType::Forest : EHexTerrainType::Mountain;;
			LogicalMap.Add(HexData);
		}
	}
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

	for (auto HexData : LogicalMap)
	{
		if (AHexActor* HexActor = GetHexActorFromPool())
		{
			HexActor->SetHexData(HexData);
			HexActor->SetMaterial(MapAssets->TerrainMaterials[HexData.TerrainType].Get());
			HexActor->GenerateMesh();
			ActiveHexActors.Add(HexActor);
		}
	}
}
