
#include "EnemyAIController.h"
#include "Base_Enemy.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

AEnemyAIController::AEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!InPawn)
    {
        return;
    }

    HomeLocation = InPawn->GetActorLocation();
    HomeRotation = InPawn->GetActorRotation();

    SetAIState(EEnemyAIState::Idle);

    WakeTimer = 0.0f;
    AttackTrackingTime = 0.0f;
    bWasInAttackState = false;
    bHasLastKnownLocation = false;

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(InPawn))
    {
        bHasWokenUp = !Enemy->UsesWakeUp();
        Enemy->SetHasWokenUp(bHasWokenUp);

        Enemy->bUseControllerRotationYaw = false;

        if (UCharacterMovementComponent* Movement =
            Enemy->GetCharacterMovement())
        {
            Movement->bOrientRotationToMovement = false;
        }

        Enemy->SetRunning(false);
        Enemy->SetAtHome(true);
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

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    PlayerTarget = PC ? PC->GetPawn() : nullptr;

    if (!Enemy->UsesWakeUp())
    {
        bHasWokenUp = true;
        Enemy->SetHasWokenUp(true);
    }

    if (!bHasWokenUp)
    {
        StopMovement();

        WakeTimer += DeltaTime;

        if (WakeTimer < Enemy->GetWakeDelay() ||
            !IsValid(PlayerTarget))
        {
            return;
        }

        const float Distance = FVector::Dist(
            Enemy->GetActorLocation(),
            PlayerTarget->GetActorLocation()
        );

        if (Distance > Enemy->GetWakeDistance())
        {
            return;
        }

        bHasWokenUp = true;
        Enemy->SetHasWokenUp(true);

        LastKnownLocation = PlayerTarget->GetActorLocation();
        bHasLastKnownLocation = true;

        StartChasing();
        return;
    }

    if (!Enemy->CanMove())
    {
        if (Enemy->IsInAttackState())
        {
            bWasInAttackState = true;

            if (IsValid(PlayerTarget))
            {
                LastKnownLocation = PlayerTarget->GetActorLocation();
                bHasLastKnownLocation = true;
            }
        }

        StopMovement();
        return;
    }

    if (bWasInAttackState)
    {
        bWasInAttackState = false;

        if (IsValid(PlayerTarget))
        {
            AttackTrackingTime = PostAttackTrackingDuration;

            LastKnownLocation = PlayerTarget->GetActorLocation();
            bHasLastKnownLocation = true;

            StartChasing();
        }
    }

    AttackTrackingTime = FMath::Max(
        0.0f,
        AttackTrackingTime - DeltaTime
    );

    if (IsValid(PlayerTarget))
    {
        if (AttackTrackingTime > 0.0f &&
            CurrentState == EEnemyAIState::Chasing)
        {
            LastKnownLocation = PlayerTarget->GetActorLocation();
            bHasLastKnownLocation = true;
        }
        else
        {
            UpdateVision();
        }
    }
    else if (CurrentState == EEnemyAIState::Chasing)
    {
        StartInvestigation();
    }

    if (!Enemy->CanMove())
    {
        StopMovement();
        return;
    }

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

    FVector Direction = PlayerLocation - EnemyLocation;
    Direction.Z = 0.0f;

    const float Distance = Direction.Size();

    if (Distance <= KINDA_SMALL_NUMBER)
    {
        return true;
    }

    const float MaxSightDistance =
        CurrentState == EEnemyAIState::Chasing
        ? LoseSightRadius
        : SightRadius;

    const FVector Forward =
        GetPawn()->GetActorForwardVector().GetSafeNormal2D();

    const float Dot = FVector::DotProduct(
        Forward,
        Direction.GetSafeNormal()
    );

    const float NormalMinimumDot = FMath::Cos(
        FMath::DegreesToRadians(VisionAngle)
    );

    const float PeripheralMinimumDot = FMath::Cos(
        FMath::DegreesToRadians(PeripheralVisionAngle)
    );

    const bool bNormalVision =
        Distance <= MaxSightDistance &&
        Dot >= NormalMinimumDot;

    const bool bPeripheralVision =
        Distance <= PeripheralSightRadius &&
        Dot >= PeripheralMinimumDot;

    if (!bNormalVision && !bPeripheralVision)
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

void AEnemyAIController::RotateTowards(
    const FVector& TargetLocation,
    float DeltaTime)
{
    ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn());

    if (!Enemy)
    {
        return;
    }

    FVector Direction =
        TargetLocation - Enemy->GetActorLocation();

    Direction.Z = 0.0f;

    if (Direction.IsNearlyZero())
    {
        return;
    }

    const FRotator NewRotation = FMath::RInterpTo(
        Enemy->GetActorRotation(),
        Direction.Rotation(),
        DeltaTime,
        ChaseTurnSpeed
    );

    Enemy->SetActorRotation(NewRotation);
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
        Enemy->SetRunning(false);
        break;
    }


    case EEnemyAIState::Chasing:
    {
        if (!IsValid(PlayerTarget))
        {
            StartInvestigation();
            break;
        }

        Enemy->SetRunning(true);

        const float Distance = FVector::Dist2D(
            Enemy->GetActorLocation(),
            PlayerTarget->GetActorLocation()
        );

        if (GetMoveStatus() == EPathFollowingStatus::Moving)
        {
            RotateAlongNavPath(DeltaTime);
        }
        else
        {
            RotateTowards(
                PlayerTarget->GetActorLocation(),
                DeltaTime
            );
        }

        if (Distance <= AttackRange &&
            (CanSeePlayer() || AttackTrackingTime > 0.0f))
        {
            Enemy->TryAttack(Cast<ACharacter>(PlayerTarget));

            if (!Enemy->CanMove())
            {
                break;
            }
        }

        if (Distance > ChaseStopDistance &&
            GetMoveStatus() != EPathFollowingStatus::Moving)
        {
            MoveToActor(
                PlayerTarget,
                ChaseStopDistance,
                false
            );
        }

        break;
    }


    case EEnemyAIState::Investigating:
    {
        Enemy->SetRunning(true);

        RotateAlongNavPath(DeltaTime);
   

        const float Distance = FVector::Dist2D(
            Enemy->GetActorLocation(),
            LastKnownLocation
        );

        if (Distance <= 120.0f ||
            StateTimer >= InvestigationTimeout)
        {
            StartSearching();
        }
        else if (GetMoveStatus() == EPathFollowingStatus::Idle)
        {
            MoveToLocation(LastKnownLocation, 85.0f);
        }

        break;
    }

    case EEnemyAIState::Searching:
    {
        Enemy->SetRunning(false);

        if (UCharacterMovementComponent* Movement =
            Enemy->GetCharacterMovement())
        {
            Movement->StopMovementImmediately();
        }

        if (StateTimer >= Enemy->GetSearchDuration())
        {
            StartReturningHome();
        }

        break;
    }

    case EEnemyAIState::ReturningHome:
    {
        Enemy->SetRunning(false);

        const float Distance = FVector::Dist2D(
            Enemy->GetActorLocation(),
            HomeLocation
        );

        if (Distance <= HomeAcceptanceRadius)
        {
            FinishReturningHome();
            break;
        }

        RotateAlongNavPath(DeltaTime);

        if (GetMoveStatus() != EPathFollowingStatus::Moving)
        {
            MoveToLocation(HomeLocation, HomeAcceptanceRadius);
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

    ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn());

    if (!Enemy)
    {
        return;
    }

    const bool bWasAtHome = Enemy->IsAtHome();

    StopMovement();
    SetAIState(EEnemyAIState::Chasing);

    Enemy->SetAtHome(false);
    Enemy->SetRunning(true);

    if (UCharacterMovementComponent* Movement =
        Enemy->GetCharacterMovement())
    {
        Movement->bOrientRotationToMovement = false;
    }

    if (bWasAtHome)
    {
        Enemy->OnPlayerSpotted();
    }

    if (Enemy->CanMove())
    {
        MoveToActor(PlayerTarget, ChaseStopDistance, false);
    }
}

void AEnemyAIController::StartInvestigation()
{
    StopMovement();

    if (!bHasLastKnownLocation)
    {
        StartReturningHome();
        return;
    }

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn()))
    {
        Enemy->SetRunning(true);
    }

    SetAIState(EEnemyAIState::Investigating);

    MoveToLocation(LastKnownLocation, 85.0f);
}

void AEnemyAIController::StartSearching()
{
    StopMovement();

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn()))
    {
        Enemy->SetRunning(false);
    }

    SetAIState(EEnemyAIState::Searching);
}

void AEnemyAIController::StartReturningHome()
{
    StopMovement();

    AttackTrackingTime = 0.0f;
    bHasLastKnownLocation = false;

    SetAIState(EEnemyAIState::ReturningHome);

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn()))
    {
        Enemy->SetRunning(false);
        Enemy->SetAtHome(false);

        if (UCharacterMovementComponent* Movement =
            Enemy->GetCharacterMovement())
        {
            Movement->bOrientRotationToMovement = false;
        }
    }

    MoveToLocation(HomeLocation, HomeAcceptanceRadius);
}

void AEnemyAIController::FinishReturningHome()
{
    StopMovement();

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn()))
    {
        Enemy->SetRunning(false);
        Enemy->SetAtHome(true);

        Enemy->SetActorRotation(HomeRotation);
    }

    SetAIState(EEnemyAIState::Idle);
}



void AEnemyAIController::RotateAlongNavPath(float DeltaTime)
{
    ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn());

    if (!Enemy)
    {
        return;
    }

    if (UPathFollowingComponent* PathFollowing =
        GetPathFollowingComponent())
    {
        if (GetMoveStatus() == EPathFollowingStatus::Moving)
        {
            RotateTowards(
                PathFollowing->GetCurrentTargetLocation(),
                DeltaTime
            );
        }
    }
}

