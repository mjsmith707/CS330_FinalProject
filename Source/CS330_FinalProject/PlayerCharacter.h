// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "GhostReplayCharacter.h"
#include "GameFramework/Character.h"
#include "PlayerCharacterMovementComponent.h"
#include "Engine.h"
#include "PlayerCharacter.generated.h"


UCLASS()
class CS330_FINALPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	UCameraComponent* FirstPersonCameraComponent;
	UPlayerCharacterMovementComponent* PCMovementComponent;

	void MoveForward(float axis);
	void MoveRight(float axis);
	void Jump();
	void StopJumping();

	// stop movement out of portals
	void StopCharacter();

	// Bridge function for TriggerVolume->CharMovementComponent
	void UpdateLocationAndRotation(FVector location, FRotator rotation);

	// Returns the current speed of the player for HUD purposes
	double getCurrentSpeed();
};
