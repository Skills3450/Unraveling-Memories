#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerupPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UPowerupDefinition;

UCLASS()
class PROJECT_MINOTAUR_API APowerupPickup : public AActor
{
	GENERATED_BODY()

public:
	APowerupPickup();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Powerup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTextRenderComponent> InteractionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPowerupDefinition> PowerupDefinition;

	bool bPlayerInRange = false;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	void Interact();
};