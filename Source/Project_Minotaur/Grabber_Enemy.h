
#pragma once

#include "CoreMinimal.h"
#include "Base_Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Grabber_Enemy.generated.h"

UCLASS()
class PROJECT_MINOTAUR_API AGrabber_Enemy : public ABase_Enemy
{
    GENERATED_BODY()

public:
    AGrabber_Enemy();

    virtual void Tick(float DeltaTime) override;
    virtual void TryAttack(ACharacter* Target) override;

    UFUNCTION(BlueprintPure, Category = "Enemy|Animation")
    bool IsGrabbing() const;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Animation")
    bool bIsGrabbing = false;

protected:
    virtual void Die_Implementation() override;

    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab", meta = (ClampMin = "0.0"))
    float GrabDuration = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab", meta = (ClampMin = "0.0"))
    float GrabHitDelay = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab", meta = (ClampMin = "0.0"))
    float GrabHoldDuration = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab", meta = (ClampMin = "0.0"))
    float GrabDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab", meta = (ClampMin = "0.0"))
    float GrabDamageDelay = 1.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab")
    float GrabRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab", meta = (ClampMin = "0.0"))
    float GrabCooldown = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab|Hitbox")
    FVector GrabHitboxExtent = FVector(50.0f, 70.0f, 80.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab|Hitbox")
    FVector GrabHitboxOffset = FVector(85.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Debug")
    bool bDrawGrabDebug = true;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> GrabbedCharacter;

    UPROPERTY()
    TObjectPtr<ACharacter> GrabTarget;

    float GrabTimeRemaining = 0.0f;
    float GrabHitTimeRemaining = 0.0f;
    float GrabHoldTimeRemaining = 0.0f;
    float GrabElapsedTime = 0.0f;
    float CooldownRemaining = 0.0f;

    bool bGrabHitChecked = false;
    bool bHasDealtGrabDamage = false;

    EMovementMode PreviousMovementMode = MOVE_Walking;
    uint8 PreviousCustomMovementMode = 0;

    void StartGrab(ACharacter* Target);
    void UpdateGrab(float DeltaTime);
    void ApplyGrabHit();
    void ApplyGrabDamage();
    void ReleaseGrabbedCharacter();
    void EndGrab();

    bool TryGrabWithHitbox(ACharacter* Target);
};
