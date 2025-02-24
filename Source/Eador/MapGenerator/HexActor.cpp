#include "HexActor.h"

#include "MapStructs.h"
#include "ProceduralMeshComponent.h"
#include "Components/TextRenderComponent.h"

#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(EAHexActor, Log, All);

#define CHANNEL(Verbosity, Format, ...) UE_LOGFMT(EAHexActor, Verbosity, Format, ##__VA_ARGS__)
#define LOG(Format, ...) CHANNEL(Log, Format, ##__VA_ARGS__)
#define WARNING(Format, ...) CHANNEL(Warning, Format, ##__VA_ARGS__)
#define ERROR(Format, ...) CHANNEL(Error, Format, ##__VA_ARGS__)

AHexActor::AHexActor()
{
	PrimaryActorTick.bCanEverTick = false;
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	SetRootComponent(ProceduralMeshComponent);

	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("Text");
	TextComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	TextComponent->SetRelativeLocation(FVector(10.0f, 10.0f, 10.0f));
	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

	Neighbors.Reserve(6);
	GlobalIdealVertices.Reserve(6);
	GlobalOffsetVertices.Reserve(6);
}

void AHexActor::BeginPlay()
{
	Super::BeginPlay();
}

void AHexActor::SetHexData(const FHexTileData& InHexData)
{
	HexData = InHexData;

	TextComponent->Text = FText::FromString(HexData.MapCoordinates.ToString());

	SetActorLocation(HexData.Position);
}

void AHexActor::FillHexNeighbors(const TMap<FIntPoint, AHexActor*>& Map)
{
	Neighbors.Empty();

	for (int q = -1; q <= 1; ++q)
	{
		for (int r = -1; r <= 1; ++r)
		{
			if (q == 0 && r == 0)
			{
				continue;
			}

			const FIntPoint Direction(q, r);
			const FIntPoint NeighborKey(HexData.MapCoordinates + Direction);

			if (AHexActor* const* NeighborActorPtr = Map.Find(NeighborKey))
			{
				Neighbors.Add(*NeighborActorPtr);
			}
		}
	}
}

TArray<FVector> AHexActor::CalculateHexVertices(float HexSize)
{
	static constexpr float RandomOffset = 30.0f;

	GlobalIdealVertices.Empty();
	GlobalOffsetVertices.Empty();

	// Углы для flat-top гекса
	TArray<FVector> LocalOffsetVertices;
	for (int32 i = 0; i < 6; ++i)
	{
		const float AngleRad = FMath::DegreesToRadians(60.0f * i);

		const FVector LocalIdealVert = FVector(
			HexSize * FMath::Cos(AngleRad), 0.0f,
			HexSize * FMath::Sin(AngleRad)
		);
		const FVector GlobalIdealVert = LocalIdealVert + HexData.Position;

		FVector GlobalOffsetVert = FVector::ZeroVector;;
		for (AHexActor* Neighbor : Neighbors)
		{
			const auto FoundIndex = Neighbor->GlobalIdealVertices.IndexOfByPredicate(
				[GlobalIdealVert](const FVector& Vert) { return Vert.Equals(GlobalIdealVert, 0.1); });
			if (FoundIndex != INDEX_NONE)
			{
				GlobalOffsetVert = Neighbor->GlobalOffsetVertices[FoundIndex];
				break;
			}
		}

		FVector LocalOffsetVert = FVector::ZeroVector;
		if (GlobalOffsetVert.IsZero())
		{
			LocalOffsetVert = LocalIdealVert + FVector(
				FMath::RandRange(-RandomOffset, RandomOffset), 0.0f,
				FMath::RandRange(-RandomOffset, RandomOffset)
			);
			GlobalOffsetVert = LocalOffsetVert + HexData.Position;
		}
		else
		{
			LocalOffsetVert = GlobalOffsetVert - HexData.Position;
		}

		LocalOffsetVertices.Add(LocalOffsetVert);
		GlobalIdealVertices.Add(GlobalIdealVert);
		GlobalOffsetVertices.Add(GlobalOffsetVert);
	}

	// Центральная вершина
	LocalOffsetVertices.Add(FVector::ZeroVector);

	return LocalOffsetVertices;
}

TArray<int32> AHexActor::GetHexTriangles()
{
	static constexpr const int32 CenterIndex = 6;

	TArray<int32> Triangles;
	// Создаем треугольники
	for (int32 i = 0; i < 6; ++i)
	{
		Triangles.Add(CenterIndex);
		Triangles.Add(i);
		Triangles.Add((i + 1) % 6);
	}

	return Triangles;
}

TArray<FVector2D> AHexActor::GetHexUVs()
{
	TArray<FVector2D> UVs;
	for (int32 i = 0; i < 6; ++i)
	{
		float AngleDeg = 60.0f * i;
		float AngleRad = FMath::DegreesToRadians(AngleDeg);
		UVs.Add(FVector2D((FMath::Cos(AngleRad) + 1.0f) / 2.0f, (FMath::Sin(AngleRad) + 1.0f) / 2.0f));
	}
	UVs.Add(FVector2D(0.5f, 0.5f)); // UV для центра

	return UVs;
}

void AHexActor::GenerateMesh(float HexSize)
{
	static const TArray<int32> Triangles = GetHexTriangles();
	static const TArray<FVector2D> UVs = GetHexUVs();

	const TArray<FVector> LocalOffsetVertices = CalculateHexVertices(HexSize);

	ProceduralMeshComponent->CreateMeshSection(0, LocalOffsetVertices, Triangles, {}, UVs, {}, {}, false);

	if (Material)
	{
		ProceduralMeshComponent->SetMaterial(0, Material);
	}
}
