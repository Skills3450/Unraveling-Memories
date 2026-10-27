
#include "PowerupComponent.h"
#include "PowerupDefinition.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

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

	for (FActivePowerup& ActivePowerup : ActivePowerups)
	{
		if (ActivePowerup.Definition == Powerup)
		{
			if (Powerup->StackMode == EPowerupStackMode::NoStack)
			{
				return;
			}

			if (ActivePowerup.Stacks < Powerup->MaxStacks)
			{
				ActivePowerup.Stacks++;
				RecalculateEffects();
			}

			return;
		}
	}

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

int32 UPowerupComponent::GetPowerupStacks(
	UPowerupDefinition* Powerup) const
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

const TArray<FActivePowerup>&
UPowerupComponent::GetActivePowerups() const
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

			const int32 Stacks = ActivePowerup.Stacks;

			switch (ActivePowerup.Definition->StackMode)
			{
			case EPowerupStackMode::NoStack:
				Result *= Effect.Strength;
				break;

			case EPowerupStackMode::Additive:
			{
				const float Bonus = Effect.Strength - 1.0f;
				Result += Bonus * Stacks;
				break;
			}

			case EPowerupStackMode::Multiplicative:
				for (int32 i = 0; i < Stacks; i++)
				{
					Result *= Effect.Strength;
				}
				break;
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

bool UPowerupComponent::TryDash()
{
	if (!bDashReady)
	{
		return false;
	}

	// The player must have an active powerup containing a Dash effect.
	float EffectStrength = 0.0f;
	bool bHasDashPowerup = false;

	for (const FActivePowerup& ActivePowerup : ActivePowerups)
	{
		if (!ActivePowerup.Definition)
		{
			continue;
		}

		for (const FPowerupEffect& Effect :
			ActivePowerup.Definition->Effects)
		{
			if (Effect.EffectType == EPowerupEffectType::Dash)
			{
				bHasDashPowerup = true;
				EffectStrength = FMath::Max(
					EffectStrength,
					Effect.Strength
				);
			}
		}
	}

	if (!bHasDashPowerup)
	{
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());

	if (!Character)
	{
		return false;
	}

	const FVector Forward = Character->GetActorForwardVector();
	const float FinalDashStrength = DashStrength * EffectStrength;

	// Apply the dash horizontally while preserving vertical velocity.
	const FVector DashVelocity(
		Forward.X * FinalDashStrength,
		Forward.Y * FinalDashStrength,
		0.0f
	);

	Character->LaunchCharacter(DashVelocity, true, false);

	bDashReady = false;

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			DashCooldownTimer,
			this,
			&UPowerupComponent::ResetDashCooldown,
			DashCooldown,
			false
		);
	}

	return true;
}

void UPowerupComponent::ResetDashCooldown()
{
	bDashReady = true;
}

void UPowerupComponent::RecalculateEffects()
{
	ApplyMovementEffects();
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

	const float SpeedMultiplier =
		GetEffectMultiplier(EPowerupEffectType::SpeedMultiplier);

	const float JumpMultiplier =
		GetEffectMultiplier(EPowerupEffectType::JumpMultiplier);

	Movement->MaxWalkSpeed =
		OriginalWalkSpeed * SpeedMultiplier;

	Movement->JumpZVelocity =
		OriginalJumpVelocity * JumpMultiplier;
}
