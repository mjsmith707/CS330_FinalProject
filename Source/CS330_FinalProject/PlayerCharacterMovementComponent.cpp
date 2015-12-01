// Fill out your copyright notice in the Description page of Project Settings.

#include "CS330_FinalProject.h"
#include "PlayerCharacterMovementComponent.h"
// reading:
// https://github.com/TheAsuro/Velocity
// http://flafla2.github.io/2015/02/14/bunnyhop.html

// docs: https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/UCharacterMovementComponent/index.html
UPlayerCharacterMovementComponent::UPlayerCharacterMovementComponent(){
	// useful variables to tweak:

	this->MaxWalkSpeed =  800.0f;
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
	
	// air control boosting zoom zoom
	// "multiplier applied to AirControl when lateral velocity is less than AirControlBoostVelocityThreshold" 
	// https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/UCharacterMovementComponent/AirControlBoostM-/index.html
	this->AirControlBoostMultiplier = 2.0f;

	// speed threshold to boost until
	// "When falling, if lateral velocity magnitude is less than this value, AirControl is multiplied by AirControlBoostMultiplier"
	// https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/UCharacterMovementComponent/AirControlBoostV-/index.html
	this->AirControlBoostVelocityThreshold = 100000.0f;

	this->BrakingDecelerationWalking = 1.0f;
	this->GroundFriction = 1.0f;

	C_AirAccelerationFriction = 0.0f;
	C_AirAcceleration = 1000.0f;
	C_GroundAcceleration = 300.0f;
	C_AirAccelerationMaxVelocity = 3000.0f;
	C_GroundAccelerationMaxVelocity = 800.0f;
	C_MinInitialVelocity = 100.0f;
	C_HopImpulse = 40000.0f;

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

	// this hit will have the normal of the colliding surface
	// nice properties of this:
	// doesn't count 'floors' (things we walk on)
	// only counts hits when we move into a wall, i.e. corresponds nicely with 'surfing' behavior
	FHitResult Hit;
	
	GetHit(Hit, HaveContact, DeltaTime);
	FVector HitSurfaceNormal = FVector(0.f, 0.f, 0.f);
	if (HaveContact){
		HitSurfaceNormal = Hit.ImpactNormal;
	}
	
	/*
	 * start doing adjustments to player trajectory here
	 */

	FVector accelDir = FVector(consumed.X, consumed.Y, consumed.Z);
	FVector prevVelocity = FVector(Velocity.X, Velocity.Y, Velocity.Z);

	AirAccelerating = !IsMovingOnGround() && IsFalling();

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

	//CalcVelocity(DeltaTime, C_AirAccelerationFriction, false, BrakingDecelerationWalking);
	//UpdateComponentVelocity();

	// autohop
	if (IsWalking()){
		AddImpulse(FVector(0.f, 0.f, C_HopImpulse));
	}

	PerformMovement(DeltaTime);
	

	GEngine->AddOnScreenDebugMessage(8, 1.f, FColor::Green, FString::Printf(TEXT("Post-slowdown Velocity: (%f, %f, %f)"), Velocity.X, Velocity.Y, Velocity.Z));

	// this performs movement based on the acceleration and velocity we're giving the controller 
	//this->PerformMovement(DeltaTime);

	// super simple 'counteract gravity acceleration' modification here, works on basic ramps without too much z-preaccel
	if (HaveContact){
		//Velocity.Z *=  0.95f;
		//Velocity.Z += -1.0f * GetGravityZ() * DeltaTime;
	}

	/*
	* end adjustments to player trajectory
	*/

	// debug info
	// accel seems to not have a z-component? Is this just gravity? Even for jumps, no z-accel at all... what about z-accel for going up ramps?
	GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Green, FString::Printf(TEXT("Acceleration: (%0.3f, %0.3f, %0.3f), Z-Gravity accel: %f"), Acceleration.X, Acceleration.Y, Acceleration.Z, GetGravityZ()));
	GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Green, FString::Printf(TEXT("Velocity: (%0.3f, %0.3f, %0.3f)"), Velocity.X, Velocity.Y, Velocity.Z));
	GEngine->AddOnScreenDebugMessage(4, 1.f, FColor::Green, FString::Printf(TEXT("Consumed input: (%0.3f, %0.3f, %0.3f)"), consumed.X, consumed.Y, consumed.Z));
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green, FString::Printf(TEXT("Falling? %d          Flying? %d          Walking? %d          IsMovingOnGround? %d          HaveContact? %d          AirAccelerating? %d"), IsFalling(), IsFlying(), IsWalking(), IsMovingOnGround(), HaveContact, AirAccelerating));
	GEngine->AddOnScreenDebugMessage(6, 1.f, HaveContact ? FColor::Green : FColor::Red, FString::Printf(TEXT("Contact normal: (%f, %f, %f)"), HitSurfaceNormal.X, HitSurfaceNormal.Y, HitSurfaceNormal.Z));
}

void UPlayerCharacterMovementComponent::GetHit(FHitResult& Hit, bool& HaveContact, float DeltaTime){
	// here, use the 'sample' way of getting a hit
	FVector consumed = GetLastInputVector();
	FVector DesiredMovementThisFrame = consumed.GetClampedToMaxSize(1.0f) * DeltaTime;
	if (!DesiredMovementThisFrame.IsNearlyZero())
	{

		// this technically moves the character twice per tick to the same place, since we use the last input vector as the consumed input vector
		// not super efficient, but I haven't found a better way to do this (also haven't put much effort into it)
		//SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			HaveContact = true;
		}
	}
}

// airstrafing code from http://flafla2.github.io/2015/02/14/bunnyhop.html
// accelDir: normalized direction that the player has requested to move (taking into account the movement keys and look direction)
// prevVelocity: The current velocity of the player, before any additional calculations
// accelerate: The server-defined player acceleration value
// max_velocity: The server-defined maximum player velocity (this is not strictly adhered to due to strafejumping)
FVector UPlayerCharacterMovementComponent::Accelerate(FVector accelDir, FVector prevVelocity, float accelerate, float max_velocity, float DeltaTime)
{
	/*
	float projVel = Vector3.Dot(prevVelocity, accelDir); // Vector projection of Current velocity onto accelDir.
	float accelVel = accelerate * Time.fixedDeltaTime; // Accelerated velocity in direction of movment

	// If necessary, truncate the accelerated velocity so the vector projection does not exceed max_velocity
	if (projVel + accelVel > max_velocity)
		accelVel = max_velocity - projVel;

	return prevVelocity + accelDir * accelVel;
	*/
	float projVel = FVector::DotProduct(prevVelocity, accelDir); // Vector projection of Current velocity onto accelDir.
	float accelVel = accelerate * DeltaTime; // Accelerated velocity in direction of movment

	// If necessary, truncate the accelerated velocity so the vector projection does not exceed max_velocity
	if (projVel + accelVel > max_velocity)
		accelVel = max_velocity - projVel;

	return prevVelocity + accelDir * accelVel;
}

FVector UPlayerCharacterMovementComponent::MoveGround(FVector accelDir, FVector prevVelocity, float DeltaTime)
{
	/*
	// Apply Friction
	float speed = prevVelocity.magnitude;
	if (speed != 0) // To avoid divide by zero errors
	{
		float drop = speed * friction * Time.fixedDeltaTime;
		prevVelocity *= Mathf.Max(speed - drop, 0) / speed; // Scale the velocity based on friction.
	}

	// ground_accelerate and max_velocity_ground are server-defined movement variables
	return Accelerate(accelDir, prevVelocity, ground_accelerate, max_velocity_ground);
	*/

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
	/*
	// air_accelerate and max_velocity_air are server-defined movement variables
	return Accelerate(accelDir, prevVelocity, air_accelerate, max_velocity_air);
	*/

	// air_accelerate and max_velocity_air are server-defined movement variables
	return Accelerate(accelDir, prevVelocity, C_AirAcceleration, C_AirAccelerationMaxVelocity, DeltaTime);
}