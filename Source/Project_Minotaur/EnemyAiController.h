
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
    Searching
};

UCLASS()
class PROJECT_MINOTAUR_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float SightRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float LoseSightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float VisionAngle = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float AttackRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float WalkSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float InvestigationTimeout = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    float SearchDuration = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    EEnemyAIState CurrentState = EEnemyAIState::Idle;

private:
    UPROPERTY()
    TObjectPtr<APawn> PlayerTarget;

    FVector LastKnownLocation = FVector::ZeroVector;

    bool bHasLastKnownLocation = false;

    float StateTimer = 0.0f;

    void UpdateVision();
    void UpdateBehavior(float DeltaTime);

    bool CanSeePlayer() const;

    void StartChasing();
    void StartInvestigation();
    void StartSearching();
    void GiveUp();

    void SetAIState(EEnemyAIState NewState);
};
