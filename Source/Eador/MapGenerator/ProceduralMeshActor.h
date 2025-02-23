// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "ProceduralMeshActor.generated.h"

UCLASS(Blueprintable, BlueprintType)
class EADOR_API AProceduralMeshActor : public AActor
{
	GENERATED_BODY()

	TArray<FVector> Vertices;

	UPROPERTY()
	UProceduralMeshComponent* ProceduralMeshComponent;

public:
	AProceduralMeshActor();

	void SetVertices(const TArray<FVector>& InVertices) { Vertices = InVertices; }

	void SetMaterial(UMaterialInterface* InMaterial) { Material = InMaterial; }

	void GenerateMesh() const;


protected:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	static TArray<int32> GetHexTriangles();
	static TArray<FVector2D> GetHexUVs();
};
