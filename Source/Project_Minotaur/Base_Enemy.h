
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Base_Enemy.generated.h"

class UBoxComponent;

UCLASS()
class PROJECT_MINOTAUR_API ABase_Enemy : public ACharacter
{
    GENERATED_BODY()

public:
    ABase_Enemy();

    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Enemy|Health")
    void TakeEnemyDamage(float DamageAmount);

    UFUNCTION(BlueprintPure, Category = "Enemy|Health")
    bool IsDead() const;

    UFUNCTION(BlueprintPure, Category = "Enemy|Animation")
    bool IsAttacking() const;

    UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
    bool IsAttackReady() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
    bool bIsAttacking = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
    bool bIsDead = false;

    UFUNCTION(BlueprintPure, Category = "Enemy|Movement")
    bool CanMove() const;

    UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
    void SetCanMove(bool bNewCanMove);

    UFUNCTION(BlueprintCallable, Category = "Enemy|Combat")
    virtual void TryAttack(ACharacter* Target);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health")
    float MaxHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Health")
    float CurrentHealth = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Movement")
    bool bCanMove = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement", meta = (ClampMin = "0.0"))
    float MoveSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
    float AttackDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
    float AttackRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
    float AttackDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
    float AttackHitDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
    float AttackCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat|Hitbox")
    FVector HitboxExtent = FVector(45.0f, 65.0f, 70.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat|Hitbox")
    FVector HitboxOffset = FVector(85.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Debug")
    bool bDrawAttackDebug = true;

    UFUNCTION(BlueprintNativeEvent, Category = "Enemy|Health")
    void Die();

    virtual void Die_Implementation();

    void StartAttackCooldown();

private:
    UPROPERTY()
    TObjectPtr<ACharacter> AttackTarget;

    UPROPERTY(Transient)
    TObjectPtr<UBoxComponent> AttackHitbox;

    float AttackTimeRemaining = 0.0f;
    float AttackHitTimeRemaining = 0.0f;
    float AttackCooldownRemaining = 0.0f;

    bool bHasAttackHit = false;

    void SpawnAttackHitbox();
    void RemoveAttackHitbox();
    void UpdateAttack(float DeltaTime);
    void ApplyAttackHit();
    void FinishAttack();
};
