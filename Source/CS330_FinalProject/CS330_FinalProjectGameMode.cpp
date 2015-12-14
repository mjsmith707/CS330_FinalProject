// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CS330_FinalProject.h"
#include "CS330_FinalProjectGameMode.h"
#include "CS330_FinalProjectHUD.h"
#include "PlayerCharacter.h"
#include "GhostReplayCharacter.h"
#include "SurfTriggerVolume.h"
#include "Kismet/GameplayStatics.h"

ACS330_FinalProjectGameMode::ACS330_FinalProjectGameMode()
	: Super()
{
	// set default pawn class
	DefaultPawnClass = APlayerCharacter::StaticClass();

	// use our custom HUD class
	HUDClass = ACS330_FinalProjectHUD::StaticClass();
}

void ACS330_FinalProjectGameMode::Tick( float DeltaSeconds ) {
	Super::Tick(DeltaSeconds);
	updateState(DeltaSeconds);
	updateHUD(DeltaSeconds);
}

void ACS330_FinalProjectGameMode::BeginPlay() {
	Super::BeginPlay();

	currentState = SurfGameState::MainMenu;
	handleTransitionLevelStart();
}

SurfGameState ACS330_FinalProjectGameMode::getCurrentState() const {
	return currentState;
}

// Per tick state update
void ACS330_FinalProjectGameMode::updateState(float deltaTime) {
	switch (currentState) {
		case SurfGameState::MainMenu: {
			handleMainMenu(deltaTime);
			break;
		}
		case SurfGameState::LevelStart: {
			handleLevelStart(deltaTime);
			break;
		}
		case SurfGameState::InSpawn: {
			handleInSpawn(deltaTime);
			break;
		}
		case SurfGameState::Running: {
			handleRunning(deltaTime);
			break;
		}
		case SurfGameState::OutOfBounds: {
			handleOutOfBounds(deltaTime);
			break;
		}
		case SurfGameState::FinishedRunning: {
			handleFinishedRunning(deltaTime);
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}

// For transitioning from one state to another
void ACS330_FinalProjectGameMode::transitionState(SurfGameState newstate) {
	switch (newstate) {
		case SurfGameState::MainMenu: {
			handleTransitionMainMenu();
			break;
		}
		case SurfGameState::LevelStart: {
			handleTransitionLevelStart();
			break;
		}
		case SurfGameState::InSpawn: {
			handleTransitionInSpawn();
			break;
		}
		case SurfGameState::Running: {
			handleTransitionRunning();
			break;
		}
		case SurfGameState::OutOfBounds: {
			handleTransitionOutOfBounds();
			break;
		}
		case SurfGameState::FinishedRunning: {
			handleTransitionFinishedRunning();
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}

// On Tick update functions for each state
void ACS330_FinalProjectGameMode::handleMainMenu(float deltaTime) {
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: MainMenu")));
}

void ACS330_FinalProjectGameMode::handleLevelStart(float deltaTime) {
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: LevelStart")));
}

void ACS330_FinalProjectGameMode::handleInSpawn(float deltaTime) {
	if (currentStage != 0) {
		runTimer += deltaTime;
	}
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: InSpawn")));
	GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Red, FString::Printf(TEXT("Stage: %d"), currentStage));
	GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Yellow, FString::Printf(TEXT("Run Time: %f"), runTimer));
	GEngine->AddOnScreenDebugMessage(11, 1.f, FColor::Yellow, FString::Printf(TEXT("Stage Time: %f"), stageTimer[currentStage]));
}

void ACS330_FinalProjectGameMode::handleRunning(float deltaTime) {
	runTimer += deltaTime;
	stageTimer[currentStage] += deltaTime;
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: Running")));
	GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Red, FString::Printf(TEXT("Stage: %d"), currentStage));
	GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Yellow, FString::Printf(TEXT("Run Time: %f"), runTimer));
	GEngine->AddOnScreenDebugMessage(11, 1.f, FColor::Yellow, FString::Printf(TEXT("Stage Time: %f"), stageTimer[currentStage]));
}

void ACS330_FinalProjectGameMode::handleOutOfBounds(float deltaTime) {
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: OutOfBounds")));
}

void ACS330_FinalProjectGameMode::handleFinishedRunning(float deltaTime) {
	if (currentStage+1 != numStages) {
		runTimer += deltaTime;
	}
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: FinishedRunning")));
	GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Red, FString::Printf(TEXT("Stage: %d"), currentStage));
	GEngine->AddOnScreenDebugMessage(10, 1.f, FColor::Yellow, FString::Printf(TEXT("Run Time: %f"), runTimer));
	GEngine->AddOnScreenDebugMessage(11, 1.f, FColor::Yellow, FString::Printf(TEXT("Stage Time: %f"), stageTimer[currentStage]));
}

// Transition logic for each state
// Transition to MainMenu
void ACS330_FinalProjectGameMode::handleTransitionMainMenu() {
	switch (currentState) {
		case SurfGameState::LevelStart: {
			// 
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}

// Transition to LevelStart
void ACS330_FinalProjectGameMode::handleTransitionLevelStart() {
	switch (currentState) {
		case SurfGameState::MainMenu: {
			currentState = SurfGameState::LevelStart;

			// Get all level spawn volumes
			// Find all trigger volumes
			TArray<AActor*> FoundActors;

			// Get all spawn volumes
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASurfTriggerVolume::StaticClass(), FoundActors);
			for (auto Actor : FoundActors) {
				ASurfTriggerVolume* volume = Cast<ASurfTriggerVolume>(Actor);
				if (volume) {
					// Add to volumes list
					if (volume->volumeType == SurfTriggerType::Spawn) {
						spawnVolumes.Add(volume);
						stageTimer.Add(0.0f);
						bestStageTimes.Add(0.0f);
						numStages++;
					}
				}
			}

			// Spawn the player
			UWorld* World = GetWorld();
			if (World) {
				// Test if thats valid. Should only trigger on an empty array
				if (!spawnVolumes.IsValidIndex(0)) {
					// Transition to Error
					GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Blue, FString::Printf(TEXT("Error: No valid spawn found for stage: %d!"), 0));
					return;
				}

				// Get the target point to spawn at
				ASurfTriggerVolume* spawn = spawnVolumes[0];
				// Spawn player
				APlayerController* pcontroller = UGameplayStatics::GetPlayerController(World, 0);
				ACharacter* pcharacter = pcontroller->GetCharacter();
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = this;
                SpawnParams.Instigator = Instigator;
                SpawnParams.bNoCollisionFail = true;
				GhostReplay = GetWorld()->SpawnActor<AGhostReplayCharacter>(spawn->GetActorLocation(), spawn->GetActorRotation(), SpawnParams);
				GhostReplay->setRecordingObject(pcharacter);
				AHUD* phud = pcontroller->GetHUD();
				player = Cast<APlayerCharacter>(pcharacter);
				hud = Cast<ACS330_FinalProjectHUD>(phud);
				if ((!player) || (!hud)) {
					// Transition to error
					GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Blue, FString::Printf(TEXT("Failed to find either the player or hud class object!")));
					return;
				}
				player->UpdateLocationAndRotation(spawn->GetActorLocation(), spawn->GetActorRotation());
			}

			// Transition to InSpawn
			handleTransitionInSpawn();
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}

// Transition to InSpawn
void ACS330_FinalProjectGameMode::handleTransitionInSpawn() {
	switch (currentState) {
		case SurfGameState::LevelStart: {
			currentState = SurfGameState::InSpawn;
			currentStage = 0;
			runTimer = 0.0f;
			stageTimer[currentStage] = 0.0f;
			break;
		}
		case SurfGameState::OutOfBounds: {
			currentState = SurfGameState::InSpawn;
			// Move Player to spawn

			// Test if thats valid. Should only trigger on an empty array
			if (!spawnVolumes.IsValidIndex(currentStage)) {
				// Transition to Error
				GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Blue, FString::Printf(TEXT("Error: No valid spawn found for stage: %d!"), currentStage));
				return;
			}

			// Get the target point to spawn at
			ASurfTriggerVolume* spawn = spawnVolumes[currentStage];
			player->UpdateLocationAndRotation(spawn->GetActorLocation(), spawn->GetActorRotation());
			break;
		}
		case SurfGameState::FinishedRunning: {
			currentState = SurfGameState::InSpawn;
			// Move to next stage
			if (lastHitSpawnStage != currentStage) {
				currentStage = currentStage+1 == numStages ? 0 : currentStage+1;
			}
			
			stageTimer[currentStage] = 0.0f;
			break;
		}
		case SurfGameState::Running: {
			currentState = SurfGameState::InSpawn;
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}

// Transition to Running
void ACS330_FinalProjectGameMode::handleTransitionRunning() {
	switch (currentState) {
		case SurfGameState::InSpawn: {
			currentState = SurfGameState::Running;
			// Reset the run timer
			if (currentStage == 0) {
				runTimer = 0.0f;
				GhostReplay->beginRecording();
				GhostReplay->beginReplaying();
			}
			// Reset the stage timer
			stageTimer[currentStage] = 0.0f;
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}

// Transition to OutOfBounds
void ACS330_FinalProjectGameMode::handleTransitionOutOfBounds() {
	switch (currentState) {
		case SurfGameState::InSpawn: {
			currentState = SurfGameState::OutOfBounds;
			// Transition back to InSpawn
			handleTransitionInSpawn();
			break;
		}
		case SurfGameState::Running: {
			currentState = SurfGameState::OutOfBounds;
			// Transition back to InSpawn
			handleTransitionInSpawn();
			break;
		}
		case SurfGameState::FinishedRunning: {
			currentState = SurfGameState::OutOfBounds;
			// Transition back to InSpawn
			handleTransitionInSpawn();
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}

// Transition to FinishedRunning
void ACS330_FinalProjectGameMode::handleTransitionFinishedRunning() {
	switch (currentState) {
		case SurfGameState::Running: {
			APlayerCharacter* Character = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			
			// Don't allow finish from the wrong stage
			if (lastHitFinishedStage != currentStage) {
				return;
			}
			currentState = SurfGameState::FinishedRunning;
			if (currentStage+1 == numStages) {
				GhostReplay->stopRecording();
				// Add to best times
				if ((bestTime > runTimer) || (bestTime == 0.0f)) {
					bestTime = runTimer;
					GhostReplay->saveRecording();
				}
			}
			// Update best stage time
			if ((bestStageTimes[currentStage] > stageTimer[currentStage]) || (bestStageTimes[currentStage] == 0)) {
				bestStageTimes[currentStage] = stageTimer[currentStage];
			}
			// Set static values for the duration of the timer
			drawStageInfo = true;
			hudStageTime = stageTimer[currentStage];
			hudStageRecord = bestStageTimes[currentStage];
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}

//unsigned int currentStage, double runTime, double runRecord, double stageTime, double stageRecord, double stageTimeComplete, double stageRecordComplete, double currentSpeed, bool drawStageInformation)
// Called per tick to update hud variables
void ACS330_FinalProjectGameMode::updateHUD(float deltaTime) {
	// Increment counter
	if (drawStageInfo) {
		stageInfoDrawTime += deltaTime;
	}
	if (stageInfoDrawTime > stageInfoDrawMaxTime) {
		drawStageInfo = false;
		stageInfoDrawTime = 0.0f;
		hudStageTime = 0.0f;
		hudStageRecord = 0.0f;
	}
	// Update HUD's variables for later drawing
	hud->setHudVariables(currentStage+1, runTimer, bestTime, stageTimer[currentStage], bestStageTimes[currentStage], hudStageTime, hudStageRecord, player->getCurrentSpeed(), drawStageInfo);
}

// Called by SurfTriggerVolume
void ACS330_FinalProjectGameMode::setLastHitSpawnStage(unsigned int stageID) {
	this->lastHitSpawnStage = stageID;
}

void ACS330_FinalProjectGameMode::setLastHitFinishedStage(unsigned int stageID) {
	this->lastHitFinishedStage = stageID;
}