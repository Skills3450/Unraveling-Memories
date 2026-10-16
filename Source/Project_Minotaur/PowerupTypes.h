#pragma once

#include "CoreMinimal.h"
#include "PowerupTypes.generated.h"

UENUM(BlueprintType)
enum class EPowerupEffectType : uint8
{
	SpeedMultiplier		UMETA(DisplayName = "Movement Speed"),
	JumpMultiplier		UMETA(DisplayName = "Jump Height"),
	DamageMultiplier	UMETA(DisplayName = "Damage"),
	HealthBonus			UMETA(DisplayName = "Max Health"),
	ShieldBonus			UMETA(DisplayName = "Shield")
};

UENUM(BlueprintType)
enum class EPowerupStackMode : uint8
{
	NoStack			UMETA(DisplayName = "No Stacking"),
	Additive		UMETA(DisplayName = "Additive"),
	Multiplicative	UMETA(DisplayName = "Multiplicative")
};

USTRUCT(BlueprintType)
struct FPowerupEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	EPowerupEffectType EffectType = EPowerupEffectType::SpeedMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	float Strength = 1.5f;
};