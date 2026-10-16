// Fill out your copyright notice in the Description page of Project Settings.


#include "PowerupPlayerCharacter.h"

// Sets default values
APowerupPlayerCharacter::APowerupPlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APowerupPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APowerupPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APowerupPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

