
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

    UFUNCTION(BlueprintPure, Category = "Enemy|Grab")
    bool IsGrabbing() const;

protected:
    virtual void Die_Implementation() override;
    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason
    ) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab")
    float GrabDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab")
    float GrabRange = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Grab")
    float GrabCooldown = 10.0f;

private:
    UPROPERTY()
    TObjectPtr<ACharacter> GrabbedCharacter;

    bool bIsGrabbing = false;

    float GrabTimeRemaining = 0.0f;
    float CooldownRemaining = 0.0f;

    EMovementMode PreviousMovementMode = MOVE_Walking;
    uint8 PreviousCustomMovementMode = 0;

    void StartGrab(ACharacter* Target);
    void UpdateGrab(float DeltaTime);
    void EndGrab();
};
