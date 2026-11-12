#include "EnemyAIController.h"
#include "Base_Enemy.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"

AEnemyAIController::AEnemyAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    bSetControlRotationFromPawnOrientation = false;
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
    SetControlRotation(FRotator(0.0f, HomeRotation.Yaw, 0.0f));

    SetAIState(EEnemyAIState::Idle);

    WakeTimer = 0.0f;
    SightingBroadcastCooldown = 0.0f;
    SharedSightingTimeRemaining = 0.0f;
    AttackTrackingTime = 0.0f;
    bWasInAttackState = false;
    bHasLastKnownLocation = false;

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(InPawn))
    {
        bHasWokenUp = !Enemy->UsesWakeUp();
        Enemy->SetHasWokenUp(bHasWokenUp);

        Enemy->bUseControllerRotationYaw = false;
        Enemy->bUseControllerRotationPitch = false;
        Enemy->bUseControllerRotationRoll = false;

        if (UCharacterMovementComponent* Movement =
            Enemy->GetCharacterMovement())
        {
            Movement->bOrientRotationToMovement = false;
            Movement->bUseControllerDesiredRotation = false;
        }

        if (USkeletalMeshComponent* Mesh = Enemy->GetMesh())
        {
            const float MinRate = FMath::Max(0.01f, MinimumAnimationRate);
            const float MaxRate = FMath::Max(MinRate, MaximumAnimationRate);
            Mesh->GlobalAnimRateScale = FMath::FRandRange(MinRate, MaxRate);
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

    SightingBroadcastCooldown = FMath::Max(0.0f, SightingBroadcastCooldown - DeltaTime);
    SharedSightingTimeRemaining = FMath::Max(0.0f, SharedSightingTimeRemaining - DeltaTime);

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
        WakeNearbyEnemies();
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
        else if (CurrentState != EEnemyAIState::Searching)
        {
            if (CurrentState != EEnemyAIState::Chasing || SharedSightingTimeRemaining <= 0.0f || CanSeePlayer())
            {
                UpdateVision();
            }
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

        if (SightingBroadcastCooldown <= 0.0f)
        {
            SightingBroadcastCooldown = 0.35f;
            BroadcastPlayerSighting();
        }

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

    FVector Direction = TargetLocation - Enemy->GetActorLocation();
    Direction.Z = 0.0f;

    if (Direction.IsNearlyZero())
    {
        return;
    }

    const float CurrentYaw = Enemy->GetActorRotation().Yaw;
    const float TargetYaw = Direction.Rotation().Yaw;
    const float NewYaw = FMath::FixedTurn(
        CurrentYaw, TargetYaw, ChaseTurnDegreesPerSecond * DeltaTime);
    Enemy->SetActorRotation(FRotator(0.0f, NewYaw, 0.0f));
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

        FVector ToPlayer = PlayerTarget->GetActorLocation() - Enemy->GetActorLocation();
        ToPlayer.Z = 0.0f;
        const float Distance = ToPlayer.Size();

        const float StopDistance = FMath::Max(0.0f, ChaseStopDistance);

        if (Distance > StopDistance)
        {
            const FVector FacingTarget = (Distance <= AttackRange + 50.0f || !bUseNavMeshChasing)
                ? PlayerTarget->GetActorLocation()
                : GetNavSteeringPoint(PlayerTarget->GetActorLocation(), DeltaTime);
            RotateTowards(FacingTarget, DeltaTime);
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

        Enemy->SetRunning(Distance > StopDistance);

        break;
    }

    case EEnemyAIState::Investigating:
    {
        const float DistanceToTarget = FVector::Dist2D(
            Enemy->GetActorLocation(), InvestigationTarget);
        const float DistanceToLastSighting = FVector::Dist2D(
            Enemy->GetActorLocation(), LastKnownLocation);

        if (DistanceToTarget <= InvestigationArrivalRadius ||
            DistanceToLastSighting <= InvestigationVicinityRadius)
        {
            StartSearching();
            break;
        }

        Enemy->SetRunning(true);
        RotateTowards(GetNavSteeringPoint(InvestigationTarget, DeltaTime), DeltaTime);
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

        if (StateTimer >= FMath::Max(1.0f, Enemy->GetSearchDuration()))
        {
            if (SharedSightingTimeRemaining > 0.0f && IsValid(PlayerTarget))
            {
                StartChasing();
            }
            else
            {
                StartReturningHome();
            }
        }

        break;
    }

    case EEnemyAIState::ReturningHome:
    {
        const float Distance = FVector::Dist2D(
            Enemy->GetActorLocation(), HomeLocation);

        if (Distance <= HomeAcceptanceRadius)
        {
            FinishReturningHome();
            break;
        }

        Enemy->SetRunning(true);
        RotateTowards(GetNavSteeringPoint(HomeLocation, DeltaTime), DeltaTime);
        break;
    }
    }
}

void AEnemyAIController::SetAIState(EEnemyAIState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn()))
    {
        if (UCharacterMovementComponent* Movement = Enemy->GetCharacterMovement())
        {
            if (CurrentState == EEnemyAIState::Searching && bSearchMovementPaused)
            {
                Movement->SetMovementMode(PreviousSearchMovementMode, PreviousSearchCustomMovementMode);
                bSearchMovementPaused = false;
            }

            if (NewState == EEnemyAIState::Searching)
            {
                PreviousSearchMovementMode = Movement->MovementMode;
                PreviousSearchCustomMovementMode = Movement->CustomMovementMode;
                Movement->StopMovementImmediately();
                Movement->DisableMovement();
                bSearchMovementPaused = true;
            }
        }
    }

    CurrentState = NewState;
    StateTimer = 0.0f;
    NavigationRefreshRemaining = 0.0f;
}

void AEnemyAIController::WakeFromNearbyEnemy(APawn* SharedTarget)
{
    ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn());
    if (!Enemy || Enemy->IsDead() || bHasWokenUp || !IsValid(SharedTarget))
    {
        return;
    }

    PlayerTarget = SharedTarget;
    bHasWokenUp = true;
    Enemy->SetHasWokenUp(true);
    WakeTimer = 0.0f;
    LastKnownLocation = SharedTarget->GetActorLocation();
    bHasLastKnownLocation = true;
    StartChasing();
}

void AEnemyAIController::ReceivePlayerSighting(
    APawn* SharedTarget,
    const FVector& ReportedLocation)
{
    ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn());
    if (!Enemy || Enemy->IsDead() || !IsValid(SharedTarget))
    {
        return;
    }

    if (!bHasWokenUp)
    {
        WakeFromNearbyEnemy(SharedTarget);
    }

    PlayerTarget = SharedTarget;
    LastKnownLocation = ReportedLocation;
    bHasLastKnownLocation = true;
    SharedSightingTimeRemaining = 1.0f;

    if (Enemy->CanMove() && CurrentState != EEnemyAIState::Chasing &&
        CurrentState != EEnemyAIState::Searching)
    {
        StartChasing();
    }
}

void AEnemyAIController::BroadcastPlayerSighting()
{
    const ABase_Enemy* SourceEnemy = Cast<ABase_Enemy>(GetPawn());
    if (!SourceEnemy || !GetWorld() || !IsValid(PlayerTarget))
    {
        return;
    }

    const FVector SourceLocation = SourceEnemy->GetActorLocation();
    const FVector ReportedLocation = PlayerTarget->GetActorLocation();
    const float RadiusSquared = FMath::Square(FMath::Max(0.0f, WakeAllyRadius));

    for (TActorIterator<ABase_Enemy> It(GetWorld()); It; ++It)
    {
        ABase_Enemy* OtherEnemy = *It;
        if (!IsValid(OtherEnemy) || OtherEnemy == SourceEnemy || OtherEnemy->IsDead() ||
            FVector::DistSquared(SourceLocation, OtherEnemy->GetActorLocation()) > RadiusSquared)
        {
            continue;
        }

        if (AEnemyAIController* OtherController = Cast<AEnemyAIController>(OtherEnemy->GetController()))
        {
            OtherController->ReceivePlayerSighting(PlayerTarget.Get(), ReportedLocation);
        }
    }
}

void AEnemyAIController::WakeNearbyEnemies()
{
    ABase_Enemy* SourceEnemy = Cast<ABase_Enemy>(GetPawn());
    if (!SourceEnemy || !IsValid(PlayerTarget) || !GetWorld())
    {
        return;
    }

    const float RadiusSquared = FMath::Square(FMath::Max(0.0f, WakeAllyRadius));
    const FVector SourceLocation = SourceEnemy->GetActorLocation();

    for (TActorIterator<ABase_Enemy> It(GetWorld()); It; ++It)
    {
        ABase_Enemy* OtherEnemy = *It;
        if (!IsValid(OtherEnemy) || OtherEnemy == SourceEnemy || OtherEnemy->IsDead())
        {
            continue;
        }

        if (FVector::DistSquared(SourceLocation, OtherEnemy->GetActorLocation()) > RadiusSquared)
        {
            continue;
        }

        if (AEnemyAIController* OtherController = Cast<AEnemyAIController>(OtherEnemy->GetController()))
        {
            OtherController->WakeFromNearbyEnemy(PlayerTarget.Get());
        }
    }
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
        Movement->bUseControllerDesiredRotation = false;
    }

    if (bWasAtHome)
    {
        Enemy->OnPlayerSpotted();
    }

}

void AEnemyAIController::ChooseInvestigationTarget()
{
    InvestigationTarget = LastKnownLocation;

    if (!bHasLastKnownLocation || !GetWorld())
    {
        return;
    }

    UNavigationSystemV1* NavSystem =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

    if (!NavSystem)
    {
        return;
    }

    const float MaxRadius = FMath::Max(0.0f, InvestigationSpreadRadius);
    const float MinimumRadius = FMath::Min(90.0f, MaxRadius);
    const float MinSeparationSquared =
        FMath::Square(FMath::Max(0.0f, InvestigationMinimumSeparation));

    for (int32 Attempt = 0; Attempt < 16; ++Attempt)
    {
        const float Angle = FMath::FRandRange(0.0f, 2.0f * PI);
        const float Radius = FMath::FRandRange(MinimumRadius, MaxRadius);
        const FVector Candidate = LastKnownLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f);

        FNavLocation Projected;
        if (!NavSystem->ProjectPointToNavigation(
            Candidate, Projected, FVector(120.0f, 120.0f, 200.0f)))
        {
            continue;
        }

        bool bTooCloseToAnotherTarget = false;
        for (TActorIterator<AEnemyAIController> It(GetWorld()); It; ++It)
        {
            const AEnemyAIController* Other = *It;
            if (!IsValid(Other) || Other == this ||
                Other->CurrentState != EEnemyAIState::Investigating ||
                FVector::DistSquared2D(Other->LastKnownLocation, LastKnownLocation) >
                FMath::Square(InvestigationSpreadRadius * 2.0f))
            {
                continue;
            }

            if (FVector::DistSquared2D(
                Projected.Location, Other->InvestigationTarget) < MinSeparationSquared)
            {
                bTooCloseToAnotherTarget = true;
                break;
            }
        }

        if (bTooCloseToAnotherTarget)
        {
            continue;
        }

        if (UNavigationPath* Path =
            UNavigationSystemV1::FindPathToLocationSynchronously(
                this, GetPawn()->GetActorLocation(), Projected.Location, GetPawn()))
        {
            if (Path->IsValid() && !Path->IsPartial())
            {
                InvestigationTarget = Projected.Location;
                return;
            }
        }
    }

    FNavLocation ProjectedLastSeen;
    if (NavSystem->ProjectPointToNavigation(
        LastKnownLocation, ProjectedLastSeen, FVector(120.0f, 120.0f, 200.0f)))
    {
        InvestigationTarget = ProjectedLastSeen.Location;
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
    ChooseInvestigationTarget();
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
        Enemy->SetRunning(true);
        Enemy->SetAtHome(false);

        if (UCharacterMovementComponent* Movement =
            Enemy->GetCharacterMovement())
        {
            Movement->bOrientRotationToMovement = false;
        }
    }

}

void AEnemyAIController::FinishReturningHome()
{
    StopMovement();

    if (ABase_Enemy* Enemy = Cast<ABase_Enemy>(GetPawn()))
    {
        Enemy->SetRunning(false);
        Enemy->SetAtHome(true);

        Enemy->SetActorRotation(FRotator(0.0f, HomeRotation.Yaw, 0.0f));
    }

    SetAIState(EEnemyAIState::Idle);
}

FVector AEnemyAIController::GetNavSteeringPoint(
    const FVector& GoalLocation,
    float DeltaTime)
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn)
    {
        return GoalLocation;
    }

    NavigationRefreshRemaining -= DeltaTime;
    const bool bGoalChanged = FVector::DistSquared2D(
        CachedNavigationGoal, GoalLocation) > FMath::Square(100.0f);

    if (NavigationRefreshRemaining <= 0.0f || bGoalChanged)
    {
        NavigationRefreshRemaining = FMath::Max(0.1f, NavigationRefreshInterval);
        CachedNavigationGoal = GoalLocation;
        CachedNavigationPoint = GoalLocation;

        if (UNavigationPath* Path = UNavigationSystemV1::FindPathToLocationSynchronously(
            this, ControlledPawn->GetActorLocation(), GoalLocation, ControlledPawn))
        {
            if (Path->IsValid() && Path->PathPoints.Num() > 1)
            {
                const FVector PawnLocation = ControlledPawn->GetActorLocation();
                for (int32 Index = 1; Index < Path->PathPoints.Num(); ++Index)
                {
                    if (FVector::DistSquared2D(Path->PathPoints[Index], PawnLocation) >
                        FMath::Square(NextWaypointAcceptanceRadius))
                    {
                        CachedNavigationPoint = Path->PathPoints[Index];
                        break;
                    }
                }
            }
        }
    }

    return CachedNavigationPoint;
}
