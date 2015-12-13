// Fill out your copyright notice in the Description page of Project Settings.

#include "CS330_FinalProject.h"
#include "GhostReplayCharacter.h"
#include "PlayerCharacter.h"

// Sets default values
AGhostReplayCharacter::AGhostReplayCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	activated = false;

	USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SetActorEnableCollision(false);
	UStaticMeshComponent* GhostVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	GhostVisual->AttachTo(RootComponent);

	//comment the second out and uncomment the following line for production
	//static ConstructorHelpers::FObjectFinder<UStaticMesh> GhostVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_NarrowCapsule"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GhostVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Cube"));

	if (GhostVisualAsset.Succeeded()){
		GhostVisual->SetStaticMesh(GhostVisualAsset.Object);
		GhostVisual->SetRelativeLocation(FVector(0, 0, -80));
		GhostVisual->SetWorldScale3D(FVector(1));
	}

	//comment all of this out to return to production lol
	static ConstructorHelpers::FObjectFinder<UMaterial> material(TEXT("/Game/OtherContent/MatrixTextures/cubeTexture_Mat"));
	if (material.Succeeded()){
		GhostVisual->SetMaterial(0, material.Object);
	}
	int i = 0;
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
	APlayerCharacter* Character = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (Character->bestArray[i] != Character->bestArray.Last()){
		this->SetActorLocation(Character->bestArray[i]);
	}
	else{
		i = 0;
		Destroy();
	}
	i++;
}

// Called to bind functionality to input
void AGhostReplayCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

void AGhostReplayCharacter::DestroyGhost(){
	Destroy();
}


