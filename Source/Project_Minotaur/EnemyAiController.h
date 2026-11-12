#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.generated.h"

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
    Idle,
    Chasing,
    Investigating,
    Searching,
    ReturningHome
};

UCLASS()
class PROJECT_MINOTAUR_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();
    virtual void Tick(float DeltaTime) override;

    void WakeFromNearbyEnemy(APawn* SharedTarget);
    void ReceivePlayerSighting(APawn* SharedTarget, const FVector& ReportedLocation);

    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
    bool IsReturningHome() const
    {
        return CurrentState == EEnemyAIState::ReturningHome;
    }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Wake", meta = (ClampMin = "0.0"))
    float WakeAllyRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Animation", meta = (ClampMin = "0.01"))
    float MinimumAnimationRate = 0.96f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Animation", meta = (ClampMin = "0.01"))
    float MaximumAnimationRate = 1.04f;

    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float SightRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float LoseSightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float VisionAngle = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float ChaseStopDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Vision")
    float PeripheralVisionAngle = 110.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Vision")
    float PeripheralSightRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float AttackRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float InvestigationArrivalRadius = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Investigation", meta = (ClampMin = "0.0"))
    float InvestigationSpreadRadius = 220.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Investigation", meta = (ClampMin = "0.0"))
    float InvestigationMinimumSeparation = 110.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Investigation", meta = (ClampMin = "0.0"))
    float InvestigationVicinityRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Navigation")
    bool bUseNavMeshChasing = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Navigation", meta = (ClampMin = "0.1"))
    float NavigationRefreshInterval = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Navigation", meta = (ClampMin = "0.0"))
    float NextWaypointAcceptanceRadius = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Navigation", meta = (ClampMin = "0.0"))
    float ChaseTurnDegreesPerSecond = 120.0f;


    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float PostAttackTrackingDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float HomeAcceptanceRadius = 75.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    EEnemyAIState CurrentState = EEnemyAIState::Idle;

private:
    UPROPERTY()
    TObjectPtr<APawn> PlayerTarget;

    FVector HomeLocation = FVector::ZeroVector;
    FRotator HomeRotation = FRotator::ZeroRotator;
    FVector LastKnownLocation = FVector::ZeroVector;
    FVector InvestigationTarget = FVector::ZeroVector;
    bool bHasLastKnownLocation = false;
    bool bWasInAttackState = false;
    float AttackTrackingTime = 0.0f;
    float StateTimer = 0.0f;
    float WakeTimer = 0.0f;
    float SightingBroadcastCooldown = 0.0f;
    float SharedSightingTimeRemaining = 0.0f;
    bool bHasWokenUp = false;

    bool bSearchMovementPaused = false;
    EMovementMode PreviousSearchMovementMode = MOVE_Walking;
    uint8 PreviousSearchCustomMovementMode = 0;

    FVector GetNavSteeringPoint(const FVector& GoalLocation, float DeltaTime);
    FVector CachedNavigationGoal = FVector::ZeroVector;
    FVector CachedNavigationPoint = FVector::ZeroVector;
    float NavigationRefreshRemaining = 0.0f;
    void RotateTowards(const FVector& TargetLocation, float DeltaTime);
    void UpdateVision();
    void UpdateBehavior(float DeltaTime);
    bool CanSeePlayer() const;
    void StartChasing();
    void WakeNearbyEnemies();
    void BroadcastPlayerSighting();
    void ChooseInvestigationTarget();
    void StartInvestigation();
    void StartSearching();
    void StartReturningHome();
    void FinishReturningHome();
    void SetAIState(EEnemyAIState NewState);
};
