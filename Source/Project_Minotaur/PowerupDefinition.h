#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PowerupTypes.h"
#include "NiagaraSystem.h"
#include "PowerupDefinition.generated.h"

class UStaticMesh;
class UTexture2D;
class USoundBase;

UCLASS(BlueprintType)
class PROJECT_MINOTAUR_API UPowerupDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup|Visual")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup|Visual")
	TObjectPtr<UStaticMesh> PickupMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup|Visual")
	TObjectPtr<UNiagaraSystem> PickupEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup|Audio")
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup|Effects")
	TArray<FPowerupEffect> Effects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup|Stacking")
	EPowerupStackMode StackMode = EPowerupStackMode::NoStack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup|Stacking", meta = (ClampMin = "1"))
	int32 MaxStacks = 1;
};