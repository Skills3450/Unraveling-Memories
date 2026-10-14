
#include "Grabber_Enemy.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AGrabber_Enemy::AGrabber_Enemy()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AGrabber_Enemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateGrab(DeltaTime);
}

void AGrabber_Enemy::TryAttack(ACharacter* Target)
{
    if (!IsValid(Target) ||
        IsDead() ||
        !CanMove() ||
        IsAttacking())
    {
        return;
    }

    const float Distance = FVector::Dist2D(
        GetActorLocation(),
        Target->GetActorLocation()
    );

    if (Distance > GrabRange)
    {
        return;
    }

    if (CooldownRemaining <= 0.0f)
    {
        StartGrab(Target);
        return;
    }

    Super::TryAttack(Target);
}


void AGrabber_Enemy::StartGrab(ACharacter* Target)
{
    if (!IsValid(Target))
    {
        return;
    }

    GrabbedCharacter = Target;
    bIsGrabbing = true;
    GrabTimeRemaining = GrabDuration;

    SetCanMove(false);

    if (UCharacterMovementComponent* Movement =
        Target->GetCharacterMovement())
    {
        PreviousMovementMode = Movement->MovementMode;
        PreviousCustomMovementMode = Movement->CustomMovementMode;

        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }
}

void AGrabber_Enemy::UpdateGrab(float DeltaTime)
{
    CooldownRemaining = FMath::Max(
        0.0f,
        CooldownRemaining - DeltaTime
    );

    if (!bIsGrabbing)
    {
        return;
    }

    GrabTimeRemaining -= DeltaTime;

    if (GrabTimeRemaining <= 0.0f)
    {
        EndGrab();
    }
}

void AGrabber_Enemy::EndGrab()
{
    if (!bIsGrabbing)
    {
        return;
    }

    if (IsValid(GrabbedCharacter))
    {
        if (UCharacterMovementComponent* Movement =
            GrabbedCharacter->GetCharacterMovement())
        {
            Movement->SetMovementMode(
                PreviousMovementMode,
                PreviousCustomMovementMode
            );
        }
    }

    GrabbedCharacter = nullptr;
    bIsGrabbing = false;

    GrabTimeRemaining = 0.0f;
    CooldownRemaining = GrabCooldown;

    SetCanMove(true);
}

bool AGrabber_Enemy::IsGrabbing() const
{
    return bIsGrabbing;
}

void AGrabber_Enemy::Die_Implementation()
{
    if (bIsGrabbing)
    {
        EndGrab();
    }

    Super::Die_Implementation();
}

void AGrabber_Enemy::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    if (bIsGrabbing)
    {
        EndGrab();
    }

    Super::EndPlay(EndPlayReason);
}
