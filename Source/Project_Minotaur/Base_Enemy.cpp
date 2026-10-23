
#include "Base_Enemy.h"
#include "AIController.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABase_Enemy::ABase_Enemy()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ABase_Enemy::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = FMath::Max(0.0f, MaxHealth);

    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
}

void ABase_Enemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateAttack(DeltaTime);

    if (bDrawAttackDebug && IsValid(AttackHitbox))
    {
        DrawDebugBox(
            GetWorld(),
            AttackHitbox->GetComponentLocation(),
            AttackHitbox->GetScaledBoxExtent(),
            AttackHitbox->GetComponentQuat(),
            FColor::Yellow,
            false,
            0.0f,
            0,
            2.0f
        );
    }
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

bool ABase_Enemy::IsDead() const
{
    return bIsDead;
}

bool ABase_Enemy::IsAttacking() const
{
    return bIsAttacking;
}

bool ABase_Enemy::IsAttackReady() const
{
    return !IsDead() &&
        CanMove() &&
        !bIsAttacking &&
        AttackCooldownRemaining <= 0.0f;
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

void ABase_Enemy::StartAttackCooldown()
{
    AttackCooldownRemaining = FMath::Max(
        0.0f,
        AttackCooldown
    );
}

void ABase_Enemy::TryAttack(ACharacter* Target)
{
    if (!IsValid(Target) || !IsAttackReady())
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

    AttackTimeRemaining = FMath::Max(
        0.0f,
        AttackDuration
    );

    AttackHitTimeRemaining = FMath::Max(
        0.0f,
        AttackHitDelay
    );

    bHasAttackHit = false;

    SetCanMove(false);
    SpawnAttackHitbox();
}

void ABase_Enemy::SpawnAttackHitbox()
{
    RemoveAttackHitbox();

    AttackHitbox = NewObject<UBoxComponent>(this);

    AttackHitbox->SetBoxExtent(HitboxExtent);
    AttackHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    AttackHitbox->SetCollisionObjectType(ECC_WorldDynamic);
    AttackHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
    AttackHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    AttackHitbox->SetGenerateOverlapEvents(true);

    AttackHitbox->SetupAttachment(GetRootComponent());
    AttackHitbox->SetRelativeLocation(HitboxOffset);
    AttackHitbox->RegisterComponent();
    AttackHitbox->UpdateOverlaps();
}

void ABase_Enemy::RemoveAttackHitbox()
{
    if (IsValid(AttackHitbox))
    {
        AttackHitbox->DestroyComponent();
    }

    AttackHitbox = nullptr;
}

void ABase_Enemy::UpdateAttack(float DeltaTime)
{
    AttackCooldownRemaining = FMath::Max(
        0.0f,
        AttackCooldownRemaining - DeltaTime
    );

    if (!bIsAttacking || IsDead())
    {
        return;
    }

    AttackTimeRemaining -= DeltaTime;
    AttackHitTimeRemaining -= DeltaTime;

    if (!bHasAttackHit &&
        AttackHitTimeRemaining <= 0.0f &&
        AttackHitDelay <= AttackDuration)
    {
        bHasAttackHit = true;
        ApplyAttackHit();
    }

    if (AttackTimeRemaining <= 0.0f)
    {
        FinishAttack();
    }
}

void ABase_Enemy::ApplyAttackHit()
{
    if (!IsValid(AttackHitbox) ||
        !IsValid(AttackTarget) ||
        IsDead())
    {
        RemoveAttackHitbox();
        return;
    }

    AttackHitbox->UpdateOverlaps();

    TArray<AActor*> OverlappingActors;
    AttackHitbox->GetOverlappingActors(OverlappingActors);

    if (OverlappingActors.Contains(AttackTarget.Get()))
    {
        const float DamageDealt = UGameplayStatics::ApplyDamage(
            AttackTarget,
            AttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );

        if (DamageDealt > 0.0f)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(
                    -1,
                    2.0f,
                    FColor::Red,
                    FString::Printf(
                        TEXT("Hit: %.0f Damage"),
                        DamageDealt
                    )
                );
            }

            if (bDrawAttackDebug)
            {
                DrawDebugBox(
                    GetWorld(),
                    AttackHitbox->GetComponentLocation(),
                    AttackHitbox->GetScaledBoxExtent(),
                    AttackHitbox->GetComponentQuat(),
                    FColor::Red,
                    false,
                    1.0f,
                    0,
                    3.0f
                );
            }
        }
    }

    RemoveAttackHitbox();
}

void ABase_Enemy::FinishAttack()
{
    if (!bIsAttacking)
    {
        return;
    }

    RemoveAttackHitbox();

    AttackTarget = nullptr;
    bIsAttacking = false;

    AttackTimeRemaining = 0.0f;
    AttackHitTimeRemaining = 0.0f;
    bHasAttackHit = false;

    StartAttackCooldown();

    if (!IsDead())
    {
        SetCanMove(true);
    }
}

void ABase_Enemy::Die_Implementation()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    bIsAttacking = false;

    AttackTarget = nullptr;

    AttackTimeRemaining = 0.0f;
    AttackHitTimeRemaining = 0.0f;
    bHasAttackHit = false;

    RemoveAttackHitbox();
    SetCanMove(false);

    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    GetCapsuleComponent()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    GetMesh()->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );
}
