#include "ProceduralMeshActor.h"


AProceduralMeshActor::AProceduralMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>("ProceduralMesh");
	RootComponent = ProceduralMeshComponent;
}

void AProceduralMeshActor::BeginPlay()
{
	Super::BeginPlay();
}

TArray<int32> AProceduralMeshActor::GetHexTriangles()
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

TArray<FVector2D> AProceduralMeshActor::GetHexUVs()
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

void AProceduralMeshActor::GenerateMesh() const
{
	if (Vertices.Num() < 7) return;

	static const TArray<int32> Triangles = GetHexTriangles();
	static const TArray<FVector2D> UVs = GetHexUVs();

	ProceduralMeshComponent->CreateMeshSection(0, Vertices, Triangles, {}, UVs, {}, {}, false);

	if (Material)
	{
		ProceduralMeshComponent->SetMaterial(0, Material);
	}
}
