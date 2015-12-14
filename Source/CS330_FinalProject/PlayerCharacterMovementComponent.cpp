// Fill out your copyright notice in the Description page of Project Settings.

#include "CS330_FinalProject.h"
#include "PlayerCharacterMovementComponent.h"
// reading:
// https://github.com/TheAsuro/Velocity
// http://flafla2.github.io/2015/02/14/bunnyhop.html

// docs: https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/UCharacterMovementComponent/index.html
UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent(){
	// useful variables to tweak:

	// not used by our custom physics code
	// this->MaxWalkSpeed =  800.0f;
	// general movement and airspeed; these should be separated into 2 different things, so should be dynamically set depending on player state
	// "The maximum ground speed when walking. Also determines maximum lateral speed when falling."

	// MaxAcceleration
	// self-described
	// "Max Acceleration (rate of change of velocity)"

	// aircontrol things
	// give full aircontrol to player
	// "amount of lateral movement control available to the character"
	// https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/UCharacterMovementComponent/AirControl/index.html
	this->AirControl = 0.0f;

	// these 2 settings shouldn't matter, since we're giving the player full aircontrol anyways
	// full aircontrol isn't great in UE4 

	// ue4 constant, not used, custom physics instead
	// air control boosting zoom zoom
	// "multiplier applied to AirControl when lateral velocity is less than AirControlBoostVelocityThreshold" 
	// https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/UCharacterMovementComponent/AirControlBoostM-/index.html
	//this->AirControlBoostMultiplier = 2.0f;

	// ue4 constant, not used, custom physics instead
	// speed threshold to boost until
	// "When falling, if lateral velocity magnitude is less than this value, AirControl is multiplied by AirControlBoostMultiplier"
	// https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/UCharacterMovementComponent/AirControlBoostV-/index.html
	// this->AirControlBoostVelocityThreshold = 100000.0f;

	// set these so the character movement component lets our character handle this (otherwise PerformMovement might
	// try affecting our velocity) 
	this->BrakingDecelerationWalking = 0.0f;
	this->GroundFriction = 0.0f;

	// carefully chosen values through playtesting
	C_InitialAirAccelerationFriction = 5.0f;
	C_InitialAirAcceleration = 30000.0f;
	C_InitialGroundAcceleration = 3200.0f;
	C_InitialAirAccelerationMaxVelocity = 150.0f;
	C_InitialGroundAccelerationMaxVelocity = 1600.0f;
	C_InitialMinInitialVelocity = 100.0f;
	C_HopImpulse = 52500.0f;

	// for tweaking
	C_AirAccelerationFriction = C_InitialAirAccelerationFriction;
	C_AirAcceleration = C_InitialAirAcceleration;
	C_GroundAcceleration = C_InitialGroundAcceleration;
	C_AirAccelerationMaxVelocity = C_InitialAirAccelerationMaxVelocity;
	C_GroundAccelerationMaxVelocity = C_InitialGroundAccelerationMaxVelocity;
	C_MinInitialVelocity = C_InitialMinInitialVelocity;

	// this and C_HopImpulse are pretty closely tied together
	this->GravityScale = 1.5f;

	// don't think this does anything 
	this->bMaintainHorizontalGroundVelocity = false;

	// slide on all the ramps
	this->SetWalkableFloorAngle(0.0f);

	// auto bunnyhopping... works for single jumps too, toggled by the player input
	AutoHop = false;

	AAccel = FVector(0.0f, 0.0f, 0.0f);
	AVel = FVector(0.f, 0.f, 0.f);
	dbgArbitraryAccel = false;
}

void UPlayerCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction){
	//if (!Acceleration.IsNearlyZero())
	//Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// material friction and restitution don't seem to affect the 'falling' behavior we're after

	// adjusting the Velocity and Acceleration fvectors directly will affect behavior of the character
	// adjusting Accel.Z has no effect (sadly), will probably be useful to track this ourselves and set Velocity.Z
	// setting Vel.Z only has an effect when the character is falling (e.g. actually falling or starting a jump)
	// IsFalling() returns true if the character is off the ground

	/*
	* get useful data here
	*/

	// consumed input vector (don't need this if propagating to the Super::TickComponent call, but if managing accel and velocity ourselves need to grab this here);
	FVector consumed = ConsumeInputVector();

	// are contacting anything (during fall)
	bool HaveContact = false;
	bool AirAccelerating = false;

	/*
	* start doing adjustments to player trajectory here
	*/

	FVector accelDir = FVector(consumed.X, consumed.Y, consumed.Z);
	FVector prevVelocity = FVector(Velocity.X, Velocity.Y, Velocity.Z);

	AirAccelerating = !IsMovingOnGround() && (IsFalling() || IsFlying());

	if (AirAccelerating){
		Velocity = MoveAir(accelDir, prevVelocity, DeltaTime);
		Acceleration = C_AirAcceleration * accelDir;
	}
	else {
		Velocity = MoveGround(accelDir, prevVelocity, DeltaTime);
		Acceleration = C_GroundAcceleration * accelDir;
	}

	// for debugging, using ue4 editor variables for accel and vel
	if (dbgArbitraryAccel){
		Acceleration = AAccel;
		Velocity = AVel;
	}

	PerformMovement(DeltaTime);

	// autohop
	if (IsWalking() && AutoHop && !IsFlying()){
		AddImpulse(FVector(0.f, 0.f, C_HopImpulse));
	}

	/*
	* end adjustments to player trajectory
	*/

	float x = (1.f / DeltaTime);
	float AdjustedDT = FMath::Pow(2.71828182845f, (-(x - 120.f) / 160.f));
	C_AirAccelerationFriction = C_InitialAirAccelerationFriction * ((x / 120.f));
	C_AirAccelerationMaxVelocity = C_InitialAirAccelerationMaxVelocity * AdjustedDT;
}


// airstrafing logic from http://flafla2.github.io/2015/02/14/bunnyhop.html
// accelDir: normalized direction that the player has requested to move (taking into account the movement keys and look direction)
// prevVelocity: The current velocity of the player, before any additional calculations
// accelerate: The server-defined player acceleration value
// max_velocity: The server-defined maximum player velocity (this is not strictly adhered to due to strafejumping)
FVector UPlayerCharacterMovementComponent::Accelerate(FVector accelDir, FVector prevVelocity, float accelerate, float max_velocity, float DeltaTime)
{
	float projVel = FVector::DotProduct(prevVelocity, accelDir); // Vector projection of Current velocity onto accelDir.
	float accelVel = accelerate * DeltaTime; // Accelerated velocity in direction of movment

	// If necessary, truncate the accelerated velocity so the vector projection does not exceed max_velocity
	if (projVel + accelVel > max_velocity)
		accelVel = max_velocity - projVel;
	return (prevVelocity + accelDir * accelVel);
}

FVector UPlayerCharacterMovementComponent::MoveGround(FVector accelDir, FVector prevVelocity, float DeltaTime)
{

	// Apply Friction
	float speed = prevVelocity.Size();//prevVelocity.magnitude;
	if (speed != 0) // To avoid divide by zero errors
	{
		float drop = speed * C_AirAccelerationFriction * DeltaTime;
		prevVelocity *= FMath::Max(speed - drop, 0.f) / speed; // Scale the velocity based on friction.
	}

	// ground_accelerate and max_velocity_ground are server-defined movement variables
	return Accelerate(accelDir, prevVelocity, C_GroundAcceleration, C_GroundAccelerationMaxVelocity, DeltaTime);
}

FVector UPlayerCharacterMovementComponent::MoveAir(FVector accelDir, FVector prevVelocity, float DeltaTime)
{
	return Accelerate(accelDir, prevVelocity, C_AirAcceleration, C_AirAccelerationMaxVelocity, DeltaTime);
}

void UPlayerCharacterMovementComponent::StartJumping(){
	AutoHop = true;
}

void UPlayerCharacterMovementComponent::StopJumping(){
	AutoHop = false;
}

double UPlayerCharacterMovementComponent::GetCurrentSpeed(){
	return Velocity.Size();
}

// slope boosting handled
// this took a lot of digging to finally find out as being the culprit behind 'falling upwards' issues
FVector UPlayerCharacterMovementComponent::HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const
{
	return SlideResult;
}

void UPlayerCharacterMovementComponent::StopCharacter(){

	Velocity.X = 0.f;
	Velocity.Y = 0.f;
	Velocity.Z = 0.f;
}