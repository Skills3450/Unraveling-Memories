
#include "Base_Enemy.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ABase_Enemy::ABase_Enemy()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ABase_Enemy::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = FMath::Max(0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

void ABase_Enemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAttack(DeltaTime);
}

void ABase_Enemy::TakeEnemyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f || IsDead())
    {
        return;
    }

    CurrentHealth = FMath::Clamp(
        CurrentHealth - DamageAmount,
        0.0f,
        FMath::Max(0.0f, MaxHealth)
    );

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

bool ABase_Enemy::IsAttacking() const
{
    return bIsAttacking;
}

bool ABase_Enemy::IsDead() const
{
    return CurrentHealth <= 0.0f;
}

bool ABase_Enemy::CanMove() const
{
    return bCanMove && !IsDead();
}

void ABase_Enemy::SetCanMove(bool bNewCanMove)
{
    bCanMove = bNewCanMove;

    if (!bCanMove)
    {
        if (AAIController* AI = Cast<AAIController>(GetController()))
        {
            AI->StopMovement();
        }
    }
}

void ABase_Enemy::TryAttack(ACharacter* Target)
{
    if (!IsValid(Target) ||
        IsDead() ||
        bIsAttacking ||
        !CanMove() ||
        AttackCooldownRemaining > 0.0f)
    {
        return;
    }

    const float Distance = FVector::Dist2D(
        GetActorLocation(),
        Target->GetActorLocation()
    );

    if (Distance > AttackRange)
    {
        return;
    }

    AttackTarget = Target;
    bIsAttacking = true;
    AttackTimeRemaining = AttackDuration;

    SetCanMove(false);
}

void ABase_Enemy::UpdateAttack(float DeltaTime)
{
    AttackCooldownRemaining = FMath::Max(
        0.0f,
        AttackCooldownRemaining - DeltaTime
    );

    if (!bIsAttacking)
    {
        return;
    }

    AttackTimeRemaining -= DeltaTime;

    if (AttackTimeRemaining <= 0.0f)
    {
        FinishAttack();
    }
}

void ABase_Enemy::FinishAttack()
{
    if (!bIsAttacking)
    {
        return;
    }

    if (IsValid(AttackTarget) && !IsDead())
    {
        const float Distance = FVector::Dist2D(
            GetActorLocation(),
            AttackTarget->GetActorLocation()
        );

        if (Distance <= AttackRange)
        {
            UGameplayStatics::ApplyDamage(
                AttackTarget,
                AttackDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
        }
    }

    AttackTarget = nullptr;
    bIsAttacking = false;
    AttackTimeRemaining = 0.0f;
    AttackCooldownRemaining = AttackCooldown;

    SetCanMove(true);
}

void ABase_Enemy::Die_Implementation()
{
    bIsAttacking = false;
    AttackTarget = nullptr;
    SetCanMove(false);
    Destroy();
}
