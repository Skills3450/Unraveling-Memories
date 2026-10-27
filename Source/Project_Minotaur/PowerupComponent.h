
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PowerupTypes.h"
#include "PowerupComponent.generated.h"

class UPowerupDefinition;

USTRUCT(BlueprintType)
struct FActivePowerup
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Powerup")
	TObjectPtr<UPowerupDefinition> Definition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Powerup")
	int32 Stacks = 1;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECT_MINOTAUR_API UPowerupComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPowerupComponent();

	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void GivePowerup(UPowerupDefinition* Powerup);

	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void RemovePowerup(UPowerupDefinition* Powerup);

	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void RemoveAllPowerups();

	UFUNCTION(BlueprintPure, Category = "Powerups")
	bool HasPowerup(UPowerupDefinition* Powerup) const;

	UFUNCTION(BlueprintPure, Category = "Powerups")
	int32 GetPowerupStacks(UPowerupDefinition* Powerup) const;

	UFUNCTION(BlueprintPure, Category = "Powerups")
	const TArray<FActivePowerup>& GetActivePowerups() const;

	UFUNCTION(BlueprintPure, Category = "Powerups")
	float GetEffectMultiplier(EPowerupEffectType EffectType) const;

	UFUNCTION(BlueprintPure, Category = "Powerups")
	float GetEffectBonus(EPowerupEffectType EffectType) const;

	// Call this from the player's input event to perform a dash.
	UFUNCTION(BlueprintCallable, Category = "Powerups|Dash")
	bool TryDash();

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TArray<FActivePowerup> ActivePowerups;

	float OriginalWalkSpeed = 0.0f;
	float OriginalJumpVelocity = 0.0f;

	// Dash settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups|Dash",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashStrength = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powerups|Dash",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float DashCooldown = 0.75f;

	bool bDashReady = true;

	FTimerHandle DashCooldownTimer;

	void ResetDashCooldown();

	void RecalculateEffects();

	void ApplyMovementEffects();
};
