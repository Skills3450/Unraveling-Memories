
#include "Grabber_Enemy.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AGrabber_Enemy::AGrabber_Enemy()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AGrabber_Enemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateGrab(DeltaTime);
}

void AGrabber_Enemy::TryAttack(ACharacter* Target)
{
    if (!IsValid(Target) ||
        !IsAttackReady() ||
        bIsGrabbing)
    {
        return;
    }

    const float Distance = FVector::Dist2D(
        GetActorLocation(),
        Target->GetActorLocation()
    );

    if (Distance > GrabRange)
    {
        return;
    }

    if (CooldownRemaining <= 0.0f &&
        FMath::RandRange(1, 3) == 1)
    {
        StartGrab(Target);
        return;
    }

    Super::TryAttack(Target);
}

void AGrabber_Enemy::StartGrab(ACharacter* Target)
{
    if (!IsValid(Target) || bIsGrabbing)
    {
        return;
    }

    GrabTarget = Target;
    GrabbedCharacter = nullptr;

    bIsGrabbing = true;
    bGrabHitChecked = false;
    bHasDealtGrabDamage = false;

    GrabElapsedTime = 0.0f;
    GrabTimeRemaining = FMath::Max(0.0f, GrabDuration);
    GrabHitTimeRemaining = FMath::Max(0.0f, GrabHitDelay);
    GrabHoldTimeRemaining = 0.0f;

    SetCanMove(false);
}

void AGrabber_Enemy::UpdateGrab(float DeltaTime)
{
    CooldownRemaining = FMath::Max(
        0.0f,
        CooldownRemaining - DeltaTime
    );

    if (!bIsGrabbing)
    {
        return;
    }

    GrabElapsedTime += DeltaTime;
    GrabTimeRemaining -= DeltaTime;

    if (!bGrabHitChecked)
    {
        GrabHitTimeRemaining -= DeltaTime;

        if (GrabHitTimeRemaining <= 0.0f)
        {
            bGrabHitChecked = true;
            ApplyGrabHit();
        }
    }

    if (!bIsGrabbing)
    {
        return;
    }

    if (bGrabHitChecked && IsValid(GrabbedCharacter))
    {
        if (!bHasDealtGrabDamage &&
            GrabElapsedTime >= GrabDamageDelay)
        {
            bHasDealtGrabDamage = true;
            ApplyGrabDamage();
        }

        GrabHoldTimeRemaining -= DeltaTime;

        if (GrabHoldTimeRemaining <= 0.0f)
        {
            ReleaseGrabbedCharacter();
        }
    }

    if (GrabTimeRemaining <= 0.0f)
    {
        EndGrab();
    }
}

void AGrabber_Enemy::ApplyGrabHit()
{
    if (!IsValid(GrabTarget) || IsDead())
    {
        EndGrab();
        return;
    }

    if (!TryGrabWithHitbox(GrabTarget.Get()))
    {
        EndGrab();
        return;
    }

    GrabbedCharacter = GrabTarget;
    GrabTarget = nullptr;

    if (UCharacterMovementComponent* Movement =
        GrabbedCharacter->GetCharacterMovement())
    {
        PreviousMovementMode = Movement->MovementMode;
        PreviousCustomMovementMode = Movement->CustomMovementMode;

        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }

    GrabHoldTimeRemaining = FMath::Max(
        0.0f,
        GrabHoldDuration
    );

    if (GrabHoldTimeRemaining <= 0.0f)
    {
        ReleaseGrabbedCharacter();
    }
}

bool AGrabber_Enemy::TryGrabWithHitbox(ACharacter* Target)
{
    if (!IsValid(Target))
    {
        return false;
    }

    UBoxComponent* GrabHitbox = NewObject<UBoxComponent>(this);

    GrabHitbox->SetBoxExtent(GrabHitboxExtent);
    GrabHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GrabHitbox->SetCollisionObjectType(ECC_WorldDynamic);
    GrabHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
    GrabHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    GrabHitbox->SetGenerateOverlapEvents(true);

    GrabHitbox->SetupAttachment(GetRootComponent());
    GrabHitbox->SetRelativeLocation(GrabHitboxOffset);
    GrabHitbox->RegisterComponent();
    GrabHitbox->UpdateOverlaps();

    TArray<AActor*> OverlappingActors;
    GrabHitbox->GetOverlappingActors(OverlappingActors);

    const bool bHit = OverlappingActors.Contains(Target);

    if (bDrawGrabDebug)
    {
        DrawDebugBox(
            GetWorld(),
            GrabHitbox->GetComponentLocation(),
            GrabHitbox->GetScaledBoxExtent(),
            GrabHitbox->GetComponentQuat(),
            bHit ? FColor::Red : FColor::Yellow,
            false,
            1.0f,
            0,
            3.0f
        );
    }

    GrabHitbox->DestroyComponent();

    return bHit;
}

void AGrabber_Enemy::ApplyGrabDamage()
{
    if (!IsValid(GrabbedCharacter) || IsDead())
    {
        return;
    }

    const float DamageDealt = UGameplayStatics::ApplyDamage(
        GrabbedCharacter,
        GrabDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    if (DamageDealt > 0.0f && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            2.0f,
            FColor::Red,
            FString::Printf(
                TEXT("Grab Hit: %.0f Damage"),
                DamageDealt
            )
        );
    }
}

void AGrabber_Enemy::ReleaseGrabbedCharacter()
{
    if (IsValid(GrabbedCharacter))
    {
        if (UCharacterMovementComponent* Movement =
            GrabbedCharacter->GetCharacterMovement())
        {
            Movement->SetMovementMode(
                PreviousMovementMode,
                PreviousCustomMovementMode
            );
        }
    }

    GrabbedCharacter = nullptr;
}

void AGrabber_Enemy::EndGrab()
{
    if (!bIsGrabbing)
    {
        return;
    }

    ReleaseGrabbedCharacter();

    GrabTarget = nullptr;
    bIsGrabbing = false;
    bGrabHitChecked = false;
    bHasDealtGrabDamage = false;

    GrabTimeRemaining = 0.0f;
    GrabHitTimeRemaining = 0.0f;
    GrabHoldTimeRemaining = 0.0f;
    GrabElapsedTime = 0.0f;

    CooldownRemaining = FMath::Max(0.0f, GrabCooldown);

    StartAttackCooldown();

    if (!IsDead())
    {
        SetCanMove(true);
    }
}

bool AGrabber_Enemy::IsGrabbing() const
{
    return bIsGrabbing;
}

void AGrabber_Enemy::Die_Implementation()
{
    if (bIsGrabbing)
    {
        EndGrab();
    }

    Super::Die_Implementation();
}

void AGrabber_Enemy::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    if (bIsGrabbing)
    {
        EndGrab();
    }

    Super::EndPlay(EndPlayReason);
}
