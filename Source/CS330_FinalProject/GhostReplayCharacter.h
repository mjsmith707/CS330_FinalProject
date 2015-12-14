// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "GhostReplayCharacter.generated.h"

UCLASS()
class CS330_FINALPROJECT_API AGhostReplayCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGhostReplayCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

private:
	// Visual Component
	UStaticMeshComponent* GhostVisual;

	// Stored location and rotation arrays
	TArray<FVector> bestLocationArray;
	TArray<FRotator> bestRotationArray;

	// Recording movement and rotation arrays
	TArray<FVector> recordingLocationArray;
	TArray<FRotator> recordingRotationArray;

	// The object to sample
	AActor* recordingObject;

	// Sampling rates
	double interpMax;
	double replayCount;
	double recordCount;
	double replayMax;

	// Whether the ghost is replaying
	bool replayActive;

	// Whether the ghost is recording
	bool recordActive;

	// Current index in the recording arrays
	unsigned int recordingIndex;

	// Current index in the replaying arrays
	unsigned int replayingIndex;

public:
	// Sets the object to record
	void setRecordingObject(AActor*);

	// Starts replaying the object
	void beginReplaying();

	// Starts recording the object
	void beginRecording();

	// Stops replaying the object
	void stopReplaying();

	// Stops recording the object
	void stopRecording();

	// Stores the recording as the best
	void saveRecording();
};
