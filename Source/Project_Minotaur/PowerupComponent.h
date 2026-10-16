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

	// Give a powerup to the owner
	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void GivePowerup(UPowerupDefinition* Powerup);

	// Remove a powerup completely
	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void RemovePowerup(UPowerupDefinition* Powerup);

	// Remove all powerups
	UFUNCTION(BlueprintCallable, Category = "Powerups")
	void RemoveAllPowerups();

	// Check if player has a specific powerup
	UFUNCTION(BlueprintPure, Category = "Powerups")
	bool HasPowerup(UPowerupDefinition* Powerup) const;

	// Get number of stacks
	UFUNCTION(BlueprintPure, Category = "Powerups")
	int32 GetPowerupStacks(UPowerupDefinition* Powerup) const;

	// Get all currently active powerups
	UFUNCTION(BlueprintPure, Category = "Powerups")
	const TArray<FActivePowerup>& GetActivePowerups() const;

	// Get a calculated multiplier for an effect
	UFUNCTION(BlueprintPure, Category = "Powerups")
	float GetEffectMultiplier(EPowerupEffectType EffectType) const;

	// Get the total bonus for an effect
	UFUNCTION(BlueprintPure, Category = "Powerups")
	float GetEffectBonus(EPowerupEffectType EffectType) const;

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TArray<FActivePowerup> ActivePowerups;

	float OriginalWalkSpeed = 0.0f;
	float OriginalJumpVelocity = 0.0f;

	void RecalculateEffects();

	void ApplyMovementEffects();
};