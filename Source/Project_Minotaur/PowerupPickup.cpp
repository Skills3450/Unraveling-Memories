#include "PowerupPickup.h"

#include "PowerupDefinition.h"
#include "PowerupComponent.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

#include "NiagaraFunctionLibrary.h"
#include "InputCoreTypes.h"

APowerupPickup::APowerupPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision =
		CreateDefaultSubobject<USphereComponent>(
			TEXT("Collision")
		);

	RootComponent = Collision;

	Collision->SetSphereRadius(150.0f);

	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("Mesh")
		);

	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractionText =
		CreateDefaultSubobject<UTextRenderComponent>(
			TEXT("InteractionText")
		);

	InteractionText->SetupAttachment(RootComponent);

	InteractionText->SetText(FText::FromString(TEXT("Press E")));
	InteractionText->SetHorizontalAlignment(EHTA_Center);
	InteractionText->SetVerticalAlignment(EVRTA_TextCenter);
	InteractionText->SetWorldSize(35.0f);
	InteractionText->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	InteractionText->SetVisibility(false);

	Collision->OnComponentBeginOverlap.AddDynamic(
		this,
		&APowerupPickup::OnOverlapBegin
	);

	Collision->OnComponentEndOverlap.AddDynamic(
		this,
		&APowerupPickup::OnOverlapEnd
	);
}

void APowerupPickup::BeginPlay()
{
	Super::BeginPlay();

	EnableInput(GetWorld()->GetFirstPlayerController());

	if (InputComponent)
	{
		InputComponent->BindKey(
			EKeys::E,
			IE_Pressed,
			this,
			&APowerupPickup::Interact
		);
	}

	if (!PowerupDefinition)
	{
		return;
	}

	if (PowerupDefinition->PickupMesh)
	{
		Mesh->SetStaticMesh(PowerupDefinition->PickupMesh);
	}

	if (PowerupDefinition->PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			PowerupDefinition->PickupEffect,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
}

void APowerupPickup::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);

	if (!Character)
	{
		return;
	}

	bPlayerInRange = true;
	InteractionText->SetVisibility(true);
}

void APowerupPickup::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);

	if (!Character)
	{
		return;
	}

	bPlayerInRange = false;
	InteractionText->SetVisibility(false);
}

void APowerupPickup::Interact()
{
	if (!bPlayerInRange)
	{
		return;
	}

	APlayerController* PlayerController =
		GetWorld()->GetFirstPlayerController();

	if (!PlayerController)
	{
		return;
	}

	ACharacter* Character =
		Cast<ACharacter>(PlayerController->GetPawn());

	if (!Character)
	{
		return;
	}

	if (!PowerupDefinition)
	{
		return;
	}

	UPowerupComponent* PowerupComponent =
		Character->FindComponentByClass<UPowerupComponent>();

	if (!PowerupComponent)
	{
		return;
	}

	PowerupComponent->GivePowerup(PowerupDefinition);

	Destroy();
}