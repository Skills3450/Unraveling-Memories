
#include "Big_Enemy.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABig_Enemy::ABig_Enemy()
{
    PrimaryActorTick.bCanEverTick = true;

    MaxHealth = 500.0f;
}

void ABig_Enemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (IsDead())
    {
        return;
    }

    UpdateSlam(DeltaTime);
}

void ABig_Enemy::TryAttack(ACharacter* Target)
{
    if (!IsValid(Target) ||
        IsDead() ||
        !CanMove() ||
        AttackState != EBigEnemyAttackState::Ready)
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

    StartSlam(Target);
}

void ABig_Enemy::StartSlam(ACharacter* Target)
{
    if (!IsValid(Target))
    {
        return;
    }

    SlamTarget = Target;

    AttackState = EBigEnemyAttackState::WindingUp;
    AttackStateTimer = SlamWindup;

    SetCanMove(false);

    if (UCharacterMovementComponent* Movement =
        GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
    }

    OnSlamWindup();
}

void ABig_Enemy::UpdateSlam(float DeltaTime)
{
    if (AttackState == EBigEnemyAttackState::Ready)
    {
        return;
    }

    AttackStateTimer -= DeltaTime;

    if (AttackStateTimer > 0.0f)
    {
        return;
    }

    switch (AttackState)
    {
    case EBigEnemyAttackState::WindingUp:
    {
        PerformSlam();
        StartRecovery();
        break;
    }

    case EBigEnemyAttackState::Recovering:
    {
        StartCooldown();
        break;
    }

    case EBigEnemyAttackState::Cooldown:
    {
        FinishCooldown();
        break;
    }

    default:
    {
        break;
    }
    }
}

void ABig_Enemy::PerformSlam()
{
    OnSlamImpact();

    if (!IsValid(SlamTarget) || IsDead())
    {
        return;
    }

    const FVector EnemyLocation = GetActorLocation();
    const FVector PlayerLocation = SlamTarget->GetActorLocation();

    const float Distance = FVector::Dist2D(
        EnemyLocation,
        PlayerLocation
    );

    const float HeightDifference = FMath::Abs(
        EnemyLocation.Z - PlayerLocation.Z
    );

    if (Distance > SlamRadius ||
        HeightDifference > SlamHeightTolerance)
    {
        return;
    }

    UGameplayStatics::ApplyDamage(
        SlamTarget,
        SlamDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );
}

void ABig_Enemy::StartRecovery()
{
    SlamTarget = nullptr;

    AttackState = EBigEnemyAttackState::Recovering;
    AttackStateTimer = SlamRecovery;

    OnSlamRecovery();
}

void ABig_Enemy::StartCooldown()
{
    AttackState = EBigEnemyAttackState::Cooldown;
    AttackStateTimer = SlamCooldown;

    SetCanMove(true);

    OnSlamFinished();
}

void ABig_Enemy::FinishCooldown()
{
    AttackState = EBigEnemyAttackState::Ready;
    AttackStateTimer = 0.0f;
}

bool ABig_Enemy::IsSlamming() const
{
    return AttackState == EBigEnemyAttackState::WindingUp ||
        AttackState == EBigEnemyAttackState::Recovering;
}

void ABig_Enemy::Die_Implementation()
{
    SlamTarget = nullptr;
    AttackState = EBigEnemyAttackState::Ready;
    AttackStateTimer = 0.0f;

    Super::Die_Implementation();
}
