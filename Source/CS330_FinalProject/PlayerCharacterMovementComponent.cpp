// Fill out your copyright notice in the Description page of Project Settings.

#include "CS330_FinalProject.h"
#include "PlayerCharacterMovementComponent.h"

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
	this->AirControl = 1.0f;

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
}	

void UPlayerCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction){
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// material friction and restitution don't seem to affect the 'falling' behavior we're after

	// adjusting the Velocity and Acceleration fvectors directly will affect behavior of the character
	// adjusting Accel.Z has no effect (sadly), will probably be useful to track this ourselves and set Velocity.Z
	// setting Vel.Z only has an effect when the character is falling (e.g. actually falling or starting a jump)
	// IsFalling() returns true if the character is off the ground

	/*
	* get useful data here
	*/

	// are contacting anything (during fall)
	bool HaveContact = false;

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
	
	// super simple 'counteract gravity acceleration' modification here, works on basic ramps without too much z-preaccel
	if (HaveContact){
		Velocity.Z *=  0.95f;
		Velocity.Z += -1.0f * GetGravityZ() * DeltaTime;
	}

	/*
	* end adjustments to player trajectory
	*/

	// debug info
	// accel seems to not have a z-component? Is this just gravity? Even for jumps, no z-accel at all... what about z-accel for going up ramps?
	GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Green, FString::Printf(TEXT("Acceleration: (%0.3f, %0.3f, %0.3f), Z-Gravity accel: %f"), Acceleration.X, Acceleration.Y, Acceleration.Z, GetGravityZ()));
	GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Green, FString::Printf(TEXT("Velocity: (%0.3f, %0.3f, %0.3f)"), Velocity.X, Velocity.Y, Velocity.Z));
	GEngine->AddOnScreenDebugMessage(4, 1.f, FColor::Green, FString::Printf(TEXT("Falling? %d          Flying? %d          IsMovingOnGround? %d          HaveContact? %d"), IsFalling(), IsFlying(), IsMovingOnGround(), HaveContact));
	GEngine->AddOnScreenDebugMessage(1, 1.f, HaveContact ? FColor::Green : FColor::Red, FString::Printf(TEXT("Contact normal: (%f, %f, %f)"), HitSurfaceNormal.X, HitSurfaceNormal.Y, HitSurfaceNormal.Z));
}

void UPlayerCharacterMovementComponent::GetHit(FHitResult& Hit, bool& HaveContact, float DeltaTime){
	// here, use the 'sample' way of getting a hit
	FVector consumed = GetLastInputVector();
	FVector DesiredMovementThisFrame = consumed.GetClampedToMaxSize(1.0f) * DeltaTime;
	if (!DesiredMovementThisFrame.IsNearlyZero())
	{

		// this technically moves the character twice per tick to the same place, since we use the last input vector as the consumed input vector
		// not super efficient, but I haven't found a better way to do this (also haven't put much effort into it)
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			HaveContact = true;
		}
	}
}


