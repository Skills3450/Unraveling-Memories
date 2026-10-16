
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Base_Enemy.generated.h"

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

    UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
    bool IsAttacking() const;

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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
    float AttackDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
    float AttackRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
    float AttackDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat")
    float AttackCooldown = 1.0f;

    UFUNCTION(BlueprintNativeEvent, Category = "Enemy|Health")
    void Die();

    virtual void Die_Implementation();

private:
    UPROPERTY()
    TObjectPtr<ACharacter> AttackTarget;

    bool bIsAttacking = false;

    float AttackTimeRemaining = 0.0f;
    float AttackCooldownRemaining = 0.0f;

    void UpdateAttack(float DeltaTime);
    void FinishAttack();
};
