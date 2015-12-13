// Fill out your copyright notice in the Description page of Project Settings.

#include "CS330_FinalProject.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
// this initializer attaches our character movement component
APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayerCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{

		// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// root component will be the capsule component, which is what gets moved etc. 
	RootComponent = GetCapsuleComponent();

	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.0f);
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// old way I used instead of initializer list
	//PlayerCharacterMovementComponent = CreateDefaultSubobject<UPlayerCharacterMovementComponent>(TEXT("PlayerCharacterMovementComponent"));
	//PlayerCharacterMovementComponent->UpdatedComponent = RootComponent;
	
	// player takes control of this character
	AutoPossessPlayer = EAutoReceiveInput::Player0;
	PCMovementComponent = (UPlayerCharacterMovementComponent*) GetMovementComponent();

	endOfRun = false;
	record = false;
}


// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	//if (record){
		AddPosToVector(FirstPersonCameraComponent->GetComponentLocation());
	//}
}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	// binds to character default behaviors
	InputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &APlayerCharacter::StopJumping);
	// binds to pawn default behaviors
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	// our movement
	InputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
}

void APlayerCharacter::MoveForward(float axis){
	if (axis != 0.0f){
		AddMovementInput(GetActorForwardVector(), axis);
	}
}

void APlayerCharacter::MoveRight(float axis){
	if (axis != 0.0f){
		AddMovementInput(GetActorRightVector(), axis);
	}
}

void APlayerCharacter::Jump(){
	PCMovementComponent->StartJumping();
}

void APlayerCharacter::StopJumping(){
	PCMovementComponent->StopJumping();
}

// Bridge function for TriggerVolume. Apparently rotation doesn't work (who knows...) but location did
void APlayerCharacter::UpdateLocationAndRotation(FVector location, FRotator rotation) {
	APlayerController* controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (controller) {
		//controller->SetActorLocationAndRotation(location, rotation);
		SetActorLocation(location);
		controller->SetControlRotation(rotation);
		
	}
}

// Returns the current speed of the player for HUD purposes
double APlayerCharacter::getCurrentSpeed() {
	return PCMovementComponent->GetCurrentSpeed();
}

void APlayerCharacter::AddPosToVector(FVector toAdd){
	movementArray.Push(toAdd);
}

void APlayerCharacter::setStartGhost(FVector &start){
	start = movementArray[0];
}

void APlayerCharacter::EndOfRun(){
	endOfRun = true;
}

void APlayerCharacter::StartGhost(){
	
	FRotator Rotation(0, 0, 0);
	GhostReplay = GetWorld()->SpawnActor<AGhostReplayCharacter>(bestArray[0], Rotation);
}

void APlayerCharacter::CopyBest(){
	bestArray = movementArray;
}