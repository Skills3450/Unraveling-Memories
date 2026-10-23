
#pragma once

#include "CoreMinimal.h"
#include "Base_Enemy.h"
#include "Big_Enemy.generated.h"

UENUM(BlueprintType)
enum class EBigEnemyAttackState : uint8
{
    Ready,
    WindingUp,
    Recovering,
    Cooldown
};

UCLASS()
class PROJECT_MINOTAUR_API ABig_Enemy : public ABase_Enemy
{
    GENERATED_BODY()

public:
    ABig_Enemy();

    virtual void Tick(float DeltaTime) override;
    virtual void TryAttack(ACharacter* Target) override;

    UFUNCTION(BlueprintPure, Category = "BigEnemy|Combat")
    bool IsSlamming() const;

protected:
    virtual void Die_Implementation() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BigEnemy|Combat")
    float SlamDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BigEnemy|Combat")
    float SlamRadius = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BigEnemy|Combat")
    float SlamWindup = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BigEnemy|Combat")
    float SlamRecovery = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BigEnemy|Combat")
    float SlamCooldown = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BigEnemy|Combat")
    float SlamHeightTolerance = 150.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BigEnemy|Combat")
    EBigEnemyAttackState AttackState = EBigEnemyAttackState::Ready;

    UFUNCTION(BlueprintImplementableEvent, Category = "BigEnemy|Animation")
    void OnSlamWindup();

    UFUNCTION(BlueprintImplementableEvent, Category = "BigEnemy|Animation")
    void OnSlamImpact();

    UFUNCTION(BlueprintImplementableEvent, Category = "BigEnemy|Animation")
    void OnSlamRecovery();

    UFUNCTION(BlueprintImplementableEvent, Category = "BigEnemy|Animation")
    void OnSlamFinished();

private:
    UPROPERTY()
    TObjectPtr<ACharacter> SlamTarget;

    float AttackStateTimer = 0.0f;

    void StartSlam(ACharacter* Target);
    void UpdateSlam(float DeltaTime);
    void PerformSlam();
    void StartRecovery();
    void StartCooldown();
    void FinishCooldown();
};
