#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerupPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
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

	// The actual powerup this pickup gives
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Powerup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPowerupDefinition> PowerupDefinition;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};