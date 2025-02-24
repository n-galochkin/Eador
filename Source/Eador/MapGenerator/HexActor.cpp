#include "HexActor.h"

#include "MapStructs.h"
#include "Components/TextRenderComponent.h"


AHexActor::AHexActor()
{
	PrimaryActorTick.bCanEverTick = false;
	TextComponent = CreateDefaultSubobject<UTextRenderComponent>("Text");
	TextComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	TextComponent->SetRelativeLocation(FVector(10.0f, 10.0f, 10.0f));
	TextComponent->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
}

void AHexActor::SetHexData(const FHexTileData& InHexData)
{
	HexData = InHexData;

	TextComponent->Text = FText::FromString(FString::Printf(TEXT("(%d:%d)"), HexData.Q, HexData.R));

	SetActorLocation(HexData.Position);
}

void AHexActor::BeginPlay()
{
	Super::BeginPlay();

}