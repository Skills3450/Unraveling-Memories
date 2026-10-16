#include "PowerupPickup.h"

#include "PowerupDefinition.h"
#include "PowerupComponent.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

#include "NiagaraFunctionLibrary.h"

APowerupPickup::APowerupPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision =
		CreateDefaultSubobject<USphereComponent>(
			TEXT("Collision")
		);

	RootComponent = Collision;

	Collision->SetSphereRadius(75.0f);

	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Mesh =
		CreateDefaultSubobject<UStaticMeshComponent>(
			TEXT("Mesh")
		);

	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Collision->OnComponentBeginOverlap.AddDynamic(
		this,
		&APowerupPickup::OnOverlapBegin
	);
}

void APowerupPickup::BeginPlay()
{
	Super::BeginPlay();

	if (!PowerupDefinition)
	{
		return;
	}

	// Set the mesh from the Data Asset
	if (PowerupDefinition->PickupMesh)
	{
		Mesh->SetStaticMesh(PowerupDefinition->PickupMesh);
	}

	// Spawn the Niagara effect from the Data Asset
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