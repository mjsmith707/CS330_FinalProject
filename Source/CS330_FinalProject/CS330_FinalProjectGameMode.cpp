// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "CS330_FinalProject.h"
#include "CS330_FinalProjectGameMode.h"
#include "CS330_FinalProjectHUD.h"
#include "PlayerCharacter.h"
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
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: InSpawn")));
	GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Blue, FString::Printf(TEXT("Time: %f"), runTimer));
}

void ACS330_FinalProjectGameMode::handleRunning(float deltaTime) {
	runTimer += deltaTime;
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: Running")));
	GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Blue, FString::Printf(TEXT("Time: %f"), runTimer));
}

void ACS330_FinalProjectGameMode::handleOutOfBounds(float deltaTime) {
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: OutOfBounds")));
}

void ACS330_FinalProjectGameMode::handleFinishedRunning(float deltaTime) {
	GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("State: FinishedRunning")));
	GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Blue, FString::Printf(TEXT("Time: %f"), runTimer));
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
					}
				}
			}

			// Spawn the player
			UWorld* World = GetWorld();
			if (World) {
				// Get random spawn volume
				int idx = FMath::RandRange(0, spawnVolumes.Num()-1);

				// Test if thats valid. Should only trigger on an empty array
				if (!spawnVolumes.IsValidIndex(idx)) {
					// Transition to Error
					return;
				}

				// Get the target point to spawn at
				ASurfTriggerVolume* spawn = spawnVolumes[idx];

				// Spawn player
				player = World->SpawnActor<APlayerCharacter>(spawn->GetActorLocation(), spawn->GetActorRotation());

				// Create controller classes
				if (player) {
					player->SpawnDefaultController();
				}
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
			runTimer = 0.0f;
			break;
		}
		case SurfGameState::OutOfBounds: {
			currentState = SurfGameState::InSpawn;
			// Move Player to spawn
			// Get random spawn volume
			int idx = FMath::RandRange(0, spawnVolumes.Num()-1);

			// Test if thats valid. Should only trigger on an empty array
			if (!spawnVolumes.IsValidIndex(idx)) {
				// Transition to Error
				return;
			}

			// Get the target point to spawn at
			ASurfTriggerVolume* spawn = spawnVolumes[idx];
			player->UpdateLocationAndRotation(spawn->GetActorLocation(), spawn->GetActorRotation());
			break;
		}
		case SurfGameState::FinishedRunning: {
			currentState = SurfGameState::InSpawn;
			runTimer = 0.0f;
			break;
		}
		case SurfGameState::Running: {
			currentState = SurfGameState::InSpawn;
			runTimer = 0.0f;
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
			// Reset timer
			runTimer = 0.0f;
			// Transition back to InSpawn
			handleTransitionInSpawn();
			break;
		}
		case SurfGameState::Running: {
			currentState = SurfGameState::OutOfBounds;
			// Reset timer
			runTimer = 0.0f;
			// Transition back to InSpawn
			handleTransitionInSpawn();
			break;
		}
		case SurfGameState::FinishedRunning: {
			currentState = SurfGameState::OutOfBounds;
			// Reset timer
			runTimer = 0.0f;
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
			currentState = SurfGameState::FinishedRunning;
			bestTimes.Add(runTimer);
			break;
		}
		case SurfGameState::UNKNOWN:
		default: {
			break;
		}
	}
}