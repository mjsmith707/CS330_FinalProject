// Fill out your copyright notice in the Description page of Project Settings.

#include "CS330_FinalProject.h"
#include "GhostReplayCharacter.h"
#include "PlayerCharacter.h"

// Sets default values
AGhostReplayCharacter::AGhostReplayCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SetActorEnableCollision(false);
	GhostVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	GhostVisual->AttachTo(RootComponent);
	GhostVisual->SetHiddenInGame(false);
	GhostVisual->SetVisibility(false);

	//comment the second out and uncomment the following line for production
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> GhostVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_NarrowCapsule"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GhostVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Cube"));

	if (GhostVisualAsset.Succeeded()){
		GhostVisual->SetStaticMesh(GhostVisualAsset.Object);
		GhostVisual->SetRelativeLocation(FVector(0, 0, -80));
		GhostVisual->SetWorldScale3D(FVector(1.5));
	}

	//comment all of this out to return to production lol
	static ConstructorHelpers::FObjectFinder<UMaterial> material(TEXT("/Game/OtherContent/MatrixTextures/cubeTexture_Mat"));
	if (material.Succeeded()){
		GhostVisual->SetMaterial(0, material.Object);
	}
	recordingObject = NULL;
	replayingIndex = 0;
	recordingIndex = 0;
	interpMax = 0.016f;
	replayCount = 0.0f;
	recordCount = 0.0f;
	replayMax = 0.0f;
}

// Called when the game starts or when spawned
void AGhostReplayCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGhostReplayCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	
	// Replays the sampled object
	if (replayActive) {
		replayCount += DeltaTime;
		if (replayCount >= interpMax) {
			replayCount = 0.0f;
			replayingIndex++;
			if (bestLocationArray.IsValidIndex(replayingIndex)) {
				SetActorLocation(bestLocationArray[replayingIndex]);
				SetActorRotation(bestRotationArray[replayingIndex]);
			}
			else {
				stopReplaying();
			}
		}
	}

	if (recordActive) {
		recordCount += DeltaTime;
		if (recordCount >= interpMax) {
			recordCount = 0.0f;
			if (recordingObject) {
				recordingLocationArray.Add(recordingObject->GetActorLocation());
				recordingRotationArray.Add(recordingObject->GetActorRotation());
			}
			else {
				recordActive = false;
			}
		}
	}
	
}

// Called to bind functionality to input
void AGhostReplayCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

void AGhostReplayCharacter::setRecordingObject(AActor* target) {
	recordingObject = target;
}

void AGhostReplayCharacter::beginReplaying() {
	replayActive = true;
	replayingIndex = 0;
	replayCount = 0.0f;
	if (bestLocationArray.IsValidIndex(replayingIndex)) {
		SetActorLocation(bestLocationArray[replayingIndex]);
		GhostVisual->SetVisibility(true);
	}
	else {
		stopReplaying();
	}

	if (bestRotationArray.IsValidIndex(replayingIndex)) {
		SetActorRotation(bestRotationArray[replayingIndex]);
	}
	else {
		stopReplaying();
	}
}

void AGhostReplayCharacter::beginRecording() {
	recordActive = true;
	recordCount = 0;
	recordingLocationArray.Empty();
	recordingRotationArray.Empty();
}

void AGhostReplayCharacter::stopReplaying() {
	replayActive = false;
	GhostVisual->SetVisibility(false);
}

void AGhostReplayCharacter::stopRecording() {
	recordActive = false;
}

void AGhostReplayCharacter::saveRecording() {
	bestLocationArray.Empty();
	bestRotationArray.Empty();
	for (auto i : recordingLocationArray) {
		bestLocationArray.Add(i);
	}
	for (auto i : recordingRotationArray) {
		bestRotationArray.Add(i);
	}
}