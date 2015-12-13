// Fill out your copyright notice in the Description page of Project Settings.

#include "CS330_FinalProject.h"
#include "PlayerCharacter.h"
#include "CS330_FinalProjectGameMode.h"
#include "SurfTriggerVolume.h"


// Sets default values
ASurfTriggerVolume::ASurfTriggerVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Trigger Box
	cubeVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeVisualRepresentation"));
    RootComponent = cubeVisual;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
    if (CubeVisualAsset.Succeeded()) {
        cubeVisual->SetStaticMesh(CubeVisualAsset.Object);
    }
	cubeVisual->bHiddenInGame = false;
	cubeVisual->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// Trigger Material
	static ConstructorHelpers::FObjectFinder<UMaterial> material(TEXT("/Game/OtherContent/M_SurfTriggerVolume.M_SurfTriggerVolume"));
    if (material.Succeeded()) {
        cubeVisual->SetMaterial(0, material.Object);
    }

	// Arrow
	arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	arrow->AttachTo(RootComponent);
	arrow->SetWorldScale3D(FVector(3.0f,1.0f,1.0f));

}

// Called when the game starts or when spawned
void ASurfTriggerVolume::BeginPlay()
{
	Super::BeginPlay();

	cubeVisual->OnComponentBeginOverlap.AddDynamic(this, &ASurfTriggerVolume::OnBeginOverlap);
}

// Called every frame
void ASurfTriggerVolume::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called when entered
void ASurfTriggerVolume::OnBeginOverlap(AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	// See if other is the player... I can't imagine how it wouldnt be unless someone knocked a box off the map
	APlayerCharacter* player = Cast<APlayerCharacter>(OtherActor);
	if (!player) {
		return;
	}

	// Handle volume logic
	switch(volumeType) {
		case SurfTriggerType::Spawn: {
			handleSpawn(player);
			break;
		}
		case SurfTriggerType::Start: {
			handleStart(player);
			break;
		}
		case SurfTriggerType::Finish: {
			handleFinish(player);
			break;
		}
		case SurfTriggerType::OutOfBounds: {
			handleOutOfBounds(player);
			break;
		}
		case SurfTriggerType::PortalEntrance: {
			handlePortalEntrance(player);
			break;
		}
		case SurfTriggerType::PortalExit: {
			handlePortalExit(player);
			break;
		}
		default: {
			// Silently fail. It's the game industry way
			break;
		}
	}
}

// Logic for if this is a spawn volume
void ASurfTriggerVolume::handleSpawn(APlayerCharacter* player) {
	// Get Game Mode
	UWorld* world = GetWorld();
	if (world) {
		AGameMode* gm = world->GetAuthGameMode();
		if (gm) {
			ACS330_FinalProjectGameMode* gamemode = Cast<ACS330_FinalProjectGameMode>(gm);
			gamemode->setLastHitSpawnStage(this->stageIndex);
			gamemode->transitionState(SurfGameState::InSpawn);
		}
	}
}

// Handle when the player starts the run (aka change gamemode state)
void ASurfTriggerVolume::handleStart(APlayerCharacter* player) {
	// Get Game Mode
	UWorld* world = GetWorld();
	if (world) {
		AGameMode* gm = world->GetAuthGameMode();
		if (gm) {
			ACS330_FinalProjectGameMode* gamemode = Cast<ACS330_FinalProjectGameMode>(gm);
			gamemode->transitionState(SurfGameState::Running);
			APlayerCharacter* Character = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
			//Character->record = true;
			if (Character->endOfRun == true){
				Character->endOfRun = false;
				Character->movementArray.Empty();
				Character->StartGhost();
			}
		}
	}
}

// Handle finishing the run (aka change gamemode state)
void ASurfTriggerVolume::handleFinish(APlayerCharacter* player) {
	// Get Game Mode
	UWorld* world = GetWorld();
	if (world) {
		AGameMode* gm = world->GetAuthGameMode();
		if (gm) {
			ACS330_FinalProjectGameMode* gamemode = Cast<ACS330_FinalProjectGameMode>(gm);
			gamemode->setLastHitFinishedStage(this->stageIndex);
			gamemode->transitionState(SurfGameState::FinishedRunning);
			// Move the player through the portal
			if (portalExit) {
				player->UpdateLocationAndRotation(portalExit->GetActorLocation(), portalExit->GetActorRotation());
			}
		}
	}
}

// Handle out of bounds (aka change gamemode state)
void ASurfTriggerVolume::handleOutOfBounds(APlayerCharacter* player) {
	// Get Game Mode
	UWorld* world = GetWorld();
	if (world) {
		AGameMode* gm = world->GetAuthGameMode();
		if (gm) {
			ACS330_FinalProjectGameMode* gamemode = Cast<ACS330_FinalProjectGameMode>(gm);
			gamemode->transitionState(SurfGameState::OutOfBounds);
		}
	}
}

// Handle the entrance to a portal (aka move the player to the exit)
void ASurfTriggerVolume::handlePortalEntrance(APlayerCharacter* player) {
	// Get Game Mode
	UWorld* world = GetWorld();
	if (world) {
		AGameMode* gm = world->GetAuthGameMode();
		if (gm) {
			ACS330_FinalProjectGameMode* gamemode = Cast<ACS330_FinalProjectGameMode>(gm);
			
			// Move the player through the portal
			if (portalExit) {
				player->UpdateLocationAndRotation(portalExit->GetActorLocation(), portalExit->GetActorRotation());
			}
			else {
				GEngine->AddOnScreenDebugMessage(7, 1.f, FColor::Red, FString::Printf(TEXT("Warning: Portal Trigger has no associated Portal Exit!")));
			}
		}
	}
}

// Probably do nothing also
void ASurfTriggerVolume::handlePortalExit(APlayerCharacter* player) {
}

