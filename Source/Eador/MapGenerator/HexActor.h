// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MapStructs.h"
#include "ProceduralMeshActor.h"
#include "HexActor.generated.h"

UCLASS(Blueprintable, BlueprintType)
class EADOR_API AHexActor : public AProceduralMeshActor
{
	GENERATED_BODY()

public:
	AHexActor();

	UPROPERTY(BlueprintReadOnly)
	FHexTileData HexData;

	UPROPERTY()
	class UTextRenderComponent* TextComponent;

	void SetHexData(const FHexTileData& InHexData);

protected:
	virtual void BeginPlay() override;
};
