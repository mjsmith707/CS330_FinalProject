// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Trigger Type
UENUM(EditAnywhere)
enum class SurfTriggerType : uint8 {
	Spawn,
	Start,
	Finish,
	OutOfBounds,
	PortalEntrance,
	PortalExit
};

class APlayerCharacter;

#include "GameFramework/Actor.h"
#include "SurfTriggerVolume.generated.h"

UCLASS()
class CS330_FINALPROJECT_API ASurfTriggerVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASurfTriggerVolume();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* cubeVisual;

	// This trigger's type
	UPROPERTY(EditAnywhere)
	SurfTriggerType volumeType;

	// Trigger Portal Link (If necessary)
	UPROPERTY(EditAnywhere)
	ASurfTriggerVolume* portalExit;

	// Triggered when something enters the box
	UFUNCTION()
	void OnBeginOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	// Volume logic functions
	void handleSpawn(APlayerCharacter* player);
	void handleStart(APlayerCharacter* player);
	void handleFinish(APlayerCharacter* player);
	void handleOutOfBounds(APlayerCharacter* player);
	void handlePortalEntrance(APlayerCharacter* player);
	void handlePortalExit(APlayerCharacter* player);
};
