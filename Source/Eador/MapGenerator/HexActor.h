// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapStructs.h"
#include "HexActor.generated.h"

UCLASS(Blueprintable, BlueprintType)
class EADOR_API AHexActor : public AActor
{
	GENERATED_BODY()

public:
	AHexActor();

	UPROPERTY(BlueprintReadOnly)
	FHexTileData HexData;

	void SetHexData(const FHexTileData& InHexData);

	void SetMaterial(UMaterialInterface* InMaterial) { Material = InMaterial; }

	void FillHexNeighbors(const TMap<FIntPoint, AHexActor*>& Map);

	void GenerateMesh(float HexSize);

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TArray<AHexActor*> Neighbors;

	UPROPERTY()
	class UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY()
	UMaterialInterface* Material;

	UPROPERTY()
	class UTextRenderComponent* TextComponent;

	TMap<FVector, FVector> GlobalIdealToOffsetVerticesMap;
	TArray<FVector> GlobalIdealVertices;
	TArray<FVector> GlobalOffsetVertices;

private:
	TArray<FVector> CalculateHexVertices(float HexSize);
	static TArray<int32> GetHexTriangles();
	static TArray<FVector2D> GetHexUVs();

};
