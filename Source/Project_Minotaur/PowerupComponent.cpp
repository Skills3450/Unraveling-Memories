#include "PowerupComponent.h"
#include "PowerupDefinition.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPowerupComponent::UPowerupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPowerupComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* Character = Cast<ACharacter>(GetOwner());

	if (Character)
	{
		UCharacterMovementComponent* Movement =
			Character->GetCharacterMovement();

		if (Movement)
		{
			OriginalWalkSpeed = Movement->MaxWalkSpeed;
			OriginalJumpVelocity = Movement->JumpZVelocity;
		}
	}
}

void UPowerupComponent::GivePowerup(UPowerupDefinition* Powerup)
{
	if (!Powerup)
	{
		return;
	}

	// Check whether the player already has this powerup
	for (FActivePowerup& ActivePowerup : ActivePowerups)
	{
		if (ActivePowerup.Definition == Powerup)
		{
			// Don't allow stacking
			if (Powerup->StackMode == EPowerupStackMode::NoStack)
			{
				return;
			}

			// Add another stack
			if (ActivePowerup.Stacks < Powerup->MaxStacks)
			{
				ActivePowerup.Stacks++;
				RecalculateEffects();
			}

			return;
		}
	}

	// New powerup
	FActivePowerup NewPowerup;
	NewPowerup.Definition = Powerup;
	NewPowerup.Stacks = 1;

	ActivePowerups.Add(NewPowerup);

	RecalculateEffects();
}

void UPowerupComponent::RemovePowerup(UPowerupDefinition* Powerup)
{
	if (!Powerup)
	{
		return;
	}

	for (int32 i = ActivePowerups.Num() - 1; i >= 0; i--)
	{
		if (ActivePowerups[i].Definition == Powerup)
		{
			ActivePowerups.RemoveAt(i);

			RecalculateEffects();

			return;
		}
	}
}

void UPowerupComponent::RemoveAllPowerups()
{
	ActivePowerups.Empty();

	RecalculateEffects();
}

bool UPowerupComponent::HasPowerup(UPowerupDefinition* Powerup) const
{
	if (!Powerup)
	{
		return false;
	}

	for (const FActivePowerup& ActivePowerup : ActivePowerups)
	{
		if (ActivePowerup.Definition == Powerup)
		{
			return true;
		}
	}

	return false;
}

int32 UPowerupComponent::GetPowerupStacks(UPowerupDefinition* Powerup) const
{
	if (!Powerup)
	{
		return 0;
	}

	for (const FActivePowerup& ActivePowerup : ActivePowerups)
	{
		if (ActivePowerup.Definition == Powerup)
		{
			return ActivePowerup.Stacks;
		}
	}

	return 0;
}

const TArray<FActivePowerup>& UPowerupComponent::GetActivePowerups() const
{
	return ActivePowerups;
}

float UPowerupComponent::GetEffectMultiplier(
	EPowerupEffectType EffectType) const
{
	float Result = 1.0f;

	for (const FActivePowerup& ActivePowerup : ActivePowerups)
	{
		if (!ActivePowerup.Definition)
		{
			continue;
		}

		for (const FPowerupEffect& Effect :
			ActivePowerup.Definition->Effects)
		{
			if (Effect.EffectType != EffectType)
			{
				continue;
			}

			int32 Stacks = ActivePowerup.Stacks;

			switch (ActivePowerup.Definition->StackMode)
			{
			case EPowerupStackMode::NoStack:
				Result *= Effect.Strength;
				break;

			case EPowerupStackMode::Additive:
			{
				float Bonus = Effect.Strength - 1.0f;
				Result += Bonus * Stacks;
				break;
			}

			case EPowerupStackMode::Multiplicative:
			{
				for (int32 i = 0; i < Stacks; i++)
				{
					Result *= Effect.Strength;
				}

				break;
			}
			}
		}
	}

	return Result;
}

float UPowerupComponent::GetEffectBonus(
	EPowerupEffectType EffectType) const
{
	float Result = 0.0f;

	for (const FActivePowerup& ActivePowerup : ActivePowerups)
	{
		if (!ActivePowerup.Definition)
		{
			continue;
		}

		for (const FPowerupEffect& Effect :
			ActivePowerup.Definition->Effects)
		{
			if (Effect.EffectType == EffectType)
			{
				Result += Effect.Strength * ActivePowerup.Stacks;
			}
		}
	}

	return Result;
}

void UPowerupComponent::RecalculateEffects()
{
	ApplyMovementEffects();

	// Other effects will use the getter functions.
	// For example:
	//
	// Damage = BaseDamage *
	// GetEffectMultiplier(EPowerupEffectType::DamageMultiplier)
	//
	// Health = BaseHealth +
	// GetEffectBonus(EPowerupEffectType::HealthBonus)
}

void UPowerupComponent::ApplyMovementEffects()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());

	if (!Character)
	{
		return;
	}

	UCharacterMovementComponent* Movement =
		Character->GetCharacterMovement();

	if (!Movement)
	{
		return;
	}

	float SpeedMultiplier =
		GetEffectMultiplier(
			EPowerupEffectType::SpeedMultiplier
		);

	float JumpMultiplier =
		GetEffectMultiplier(
			EPowerupEffectType::JumpMultiplier
		);

	Movement->MaxWalkSpeed =
		OriginalWalkSpeed * SpeedMultiplier;

	Movement->JumpZVelocity =
		OriginalJumpVelocity * JumpMultiplier;
}