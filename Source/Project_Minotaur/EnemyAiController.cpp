
#include "EnemyAIController.h"
#include "Base_Enemy.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"

AEnemyAIController::AEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(InPawn))
    {
        if (UCharacterMovementComponent* Movement =
            Enemy->GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = WalkSpeed;
            Movement->bOrientRotationToMovement = true;
        }

        Enemy->bUseControllerRotationYaw = false;
    }
}

void AEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn());

    if (!Enemy || Enemy->IsDead())
    {
        StopMovement();
        return;
    }

    if (!Enemy->CanMove())
    {
        StopMovement();
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();

    PlayerTarget = PC ? PC->GetPawn() : nullptr;

    if (!IsValid(PlayerTarget))
    {
        if (CurrentState == EEnemyAIState::Chasing)
        {
            StartInvestigation();
        }

        UpdateBehavior(DeltaTime);
        return;
    }

    UpdateVision();
    UpdateBehavior(DeltaTime);
}

bool AEnemyAIController::CanSeePlayer() const
{
    if (!IsValid(PlayerTarget) || !GetPawn())
    {
        return false;
    }

    const FVector EnemyLocation = GetPawn()->GetActorLocation();
    const FVector PlayerLocation = PlayerTarget->GetActorLocation();

    const FVector Direction = PlayerLocation - EnemyLocation;

    const float Distance = Direction.Size();

    const float MaxDistance =
        CurrentState == EEnemyAIState::Chasing
        ? LoseSightRadius
        : SightRadius;

    if (Distance > MaxDistance)
    {
        return false;
    }

    if (Distance <= KINDA_SMALL_NUMBER)
    {
        return true;
    }

    const FVector Forward = GetPawn()->GetActorForwardVector();

    const float Dot = FVector::DotProduct(
        Forward.GetSafeNormal(),
        Direction.GetSafeNormal()
    );

    const float MinimumDot = FMath::Cos(
        FMath::DegreesToRadians(VisionAngle)
    );

    if (Dot < MinimumDot)
    {
        return false;
    }

    return LineOfSightTo(PlayerTarget);
}

void AEnemyAIController::UpdateVision()
{
    if (CanSeePlayer())
    {
        LastKnownLocation = PlayerTarget->GetActorLocation();
        bHasLastKnownLocation = true;

        if (CurrentState != EEnemyAIState::Chasing)
        {
            StartChasing();
        }
    }
    else if (CurrentState == EEnemyAIState::Chasing)
    {
        StartInvestigation();
    }
}

void AEnemyAIController::UpdateBehavior(float DeltaTime)
{
    StateTimer += DeltaTime;

    ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn());

    if (!Enemy || !Enemy->CanMove())
    {
        return;
    }

    switch (CurrentState)
    {
    case EEnemyAIState::Idle:
    {
        break;
    }

    case EEnemyAIState::Chasing:
    {
        if (!IsValid(PlayerTarget))
        {
            StartInvestigation();
            break;
        }

        const float Distance = FVector::Dist2D(
            Enemy->GetActorLocation(),
            PlayerTarget->GetActorLocation()
        );

        if (Distance <= AttackRange && CanSeePlayer())
        {
            Enemy->TryAttack(Cast<ACharacter>(PlayerTarget));

            if (!Enemy->CanMove())
            {
                break;
            }
        }

        if (Distance > AttackRange &&
            GetMoveStatus() == EPathFollowingStatus::Idle)
        {
            MoveToActor(PlayerTarget, AttackRange, false);
        }

        break;
    }

    case EEnemyAIState::Investigating:
    {
        const float Distance = FVector::Dist2D(
            Enemy->GetActorLocation(),
            LastKnownLocation
        );

        if (Distance <= 120.0f ||
            StateTimer >= InvestigationTimeout)
        {
            StartSearching();
        }

        break;
    }

    case EEnemyAIState::Searching:
    {
        if (StateTimer >= SearchDuration)
        {
            GiveUp();
        }

        break;
    }
    }
}

void AEnemyAIController::SetAIState(EEnemyAIState NewState)
{
    CurrentState = NewState;
    StateTimer = 0.0f;
}

void AEnemyAIController::StartChasing()
{
    if (!IsValid(PlayerTarget))
    {
        return;
    }

    SetAIState(EEnemyAIState::Chasing);

    MoveToActor(PlayerTarget, AttackRange, false);
}

void AEnemyAIController::StartInvestigation()
{
    StopMovement();

    if (!bHasLastKnownLocation)
    {
        GiveUp();
        return;
    }

    SetAIState(EEnemyAIState::Investigating);

    MoveToLocation(LastKnownLocation, 85.0f);
}

void AEnemyAIController::StartSearching()
{
    StopMovement();

    SetAIState(EEnemyAIState::Searching);
}

void AEnemyAIController::GiveUp()
{
    StopMovement();

    bHasLastKnownLocation = false;

    SetAIState(EEnemyAIState::Idle);
}
