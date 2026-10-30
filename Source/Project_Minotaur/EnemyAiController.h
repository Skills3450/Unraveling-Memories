
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
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

    UFUNCTION(BlueprintPure, Category = "Enemy|AI")
    bool IsReturningHome() const
    {
        return CurrentState == EEnemyAIState::ReturningHome;
    }

protected:
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Vision")
    float SightRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Vision")
    float LoseSightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Vision")
    float VisionAngle = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float ChaseStopDistance = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Vision")
    float PeripheralVisionAngle = 110.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI|Vision")
    float PeripheralSightRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float AttackRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float InvestigationTimeout = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float HomeAcceptanceRadius = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float ChaseTurnSpeed = 5.0f;



    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float PostAttackTrackingDuration = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    EEnemyAIState CurrentState = EEnemyAIState::Idle;

private:
    UPROPERTY()
    TObjectPtr<APawn> PlayerTarget;

    FVector HomeLocation = FVector::ZeroVector;
    FRotator HomeRotation = FRotator::ZeroRotator;
    FVector LastKnownLocation = FVector::ZeroVector;

    bool bHasLastKnownLocation = false;
    bool bWasInAttackState = false;
    bool bHasWokenUp = false;

    float StateTimer = 0.0f;
    float WakeTimer = 0.0f;
    float AttackTrackingTime = 0.0f;
    void RotateAlongNavPath(float DeltaTime);
    void UpdateVision();
    void UpdateBehavior(float DeltaTime);
    void RotateTowards(const FVector& TargetLocation, float DeltaTime);

    bool CanSeePlayer() const;

    void StartChasing();
    void StartInvestigation();
    void StartSearching();
    void StartReturningHome();
    void FinishReturningHome();

    void SetAIState(EEnemyAIState NewState);
};
