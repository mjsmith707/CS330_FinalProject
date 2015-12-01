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

	this->BrakingDecelerationWalking = 0.0f;
	this->GroundFriction = 0.0f;

	// carefully chosen values
	C_AirAccelerationFriction = 5.0f;
	C_AirAcceleration = 30000.0f;
	C_GroundAcceleration = 3200.0f;
	C_AirAccelerationMaxVelocity = 150.0f;
	C_GroundAccelerationMaxVelocity = 1600.0f;
	C_MinInitialVelocity = 100.0f;
	C_HopImpulse = 40000.0f;

	this->bMaintainHorizontalGroundVelocity = false;
	// slide on all the ramps
	this->SetWalkableFloorAngle(0.0f);

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

	// this hit will have the normal of the colliding surface
	// nice properties of this:
	// doesn't count 'floors' (things we walk on)
	// only counts hits when we move into a wall, i.e. corresponds nicely with 'surfing' behavior
	FHitResult Hit;
	
	GetHit(Hit, HaveContact, DeltaTime);
	FVector HitSurfaceNormal = FVector(0.f, 0.f, 0.f);
	
	/*
	 * start doing adjustments to player trajectory here
	 */

	FVector accelDir = FVector(consumed.X, consumed.Y, consumed.Z);
	FVector prevVelocity = FVector(Velocity.X, Velocity.Y, Velocity.Z);

	AirAccelerating = !IsMovingOnGround() && (IsFalling() ||  IsFlying());

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

	// autohop
	if (IsWalking() && AutoHop && !IsFlying()){
		// enter custom physics mode when hopping
		// enable this when working on custom mode
		// SetMovementMode(MOVE_Custom, 0);
		AddImpulse(FVector(0.f, 0.f, C_HopImpulse));
	}

	// doesn't work yet 
	/*
	else if (IsMovingOnGround()){
		AdjustFloorHeight();
		SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
	}
	else if (MovementMode == MOVE_Custom) {
		if (CurrentFloor.FloorDist <= MIN_FLOOR_DIST)
		{
		
			SetMovementMode(MOVE_Walking);
		}
	}
	*/

	PerformMovement(DeltaTime);
	

	GEngine->AddOnScreenDebugMessage(8, 1.f, FColor::Green, FString::Printf(TEXT("Post-slowdown Velocity: (%f, %f, %f)"), Velocity.X, Velocity.Y, Velocity.Z));


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
		SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			HaveContact = true;
			//SlideAlongSurface(DesiredMovementThisFrame, 1.f - Hit.Time, Hit.Normal, Hit, false);
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
		
	return (prevVelocity + accelDir * accelVel);
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

void UPlayerCharacterMovementComponent::StartJumping(){
	AutoHop = true;
}

void UPlayerCharacterMovementComponent::StopJumping(){
	AutoHop = false;
}

void UPlayerCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations){
	// custom physics mode going to go here
	// walk mode placeholder for now
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, FString::Printf(TEXT("Init2winit")));

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	FVector FallAcceleration = GetFallingLateralAcceleration(deltaTime);
	FallAcceleration.Z = 0.f;
	const bool bHasAirControl = (FallAcceleration.SizeSquared2D() > 0.f);

	float remainingTime = deltaTime;
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
	{
		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
		bJustTeleported = false;

		FVector OldVelocity = Velocity;
		FVector VelocityNoAirControl = Velocity;

		// Apply input
		if (!HasRootMotion())
		{
			// Compute VelocityNoAirControl
			if (bHasAirControl)
			{
				// Find velocity *without* acceleration.
				TGuardValue<FVector> RestoreAcceleration(Acceleration, FVector::ZeroVector);
				TGuardValue<FVector> RestoreVelocity(Velocity, Velocity);
				Velocity.Z = 0.f;
				CalcVelocity(timeTick, FallingLateralFriction, false, BrakingDecelerationFalling);
				VelocityNoAirControl = FVector(Velocity.X, Velocity.Y, OldVelocity.Z);
			}

			// Compute Velocity
			{
				// Acceleration = FallAcceleration for CalcVelocity(), but we restore it after using it.
				TGuardValue<FVector> RestoreAcceleration(Acceleration, FallAcceleration);
				Velocity.Z = 0.f;
				CalcVelocity(timeTick, FallingLateralFriction, false, BrakingDecelerationFalling);
				Velocity.Z = OldVelocity.Z;
			}

			// Just copy Velocity to VelocityNoAirControl if they are the same (ie no acceleration).
			if (!bHasAirControl)
			{
				VelocityNoAirControl = Velocity;
			}
		}

		// Apply gravity
		const FVector Gravity(0.f, 0.f, GetGravityZ());
		Velocity = NewFallVelocity(Velocity, Gravity, timeTick);
		VelocityNoAirControl = NewFallVelocity(VelocityNoAirControl, Gravity, timeTick);
		const FVector AirControlAccel = (Velocity - VelocityNoAirControl) / timeTick;

		if (bNotifyApex && CharacterOwner->Controller && (Velocity.Z <= 0.f))
		{
			// Just passed jump apex since now going down
			bNotifyApex = false;
			NotifyJumpApex();
		}


		// Move
		FHitResult Hit(1.f);
		FVector Adjusted = 0.5f*(OldVelocity + Velocity) * timeTick;
		SafeMoveUpdatedComponent(Adjusted, PawnRotation, true, Hit);

		if (!HasValidData())
		{
			return;
		}

		float LastMoveTimeSlice = timeTick;
		float subTimeTickRemaining = timeTick * (1.f - Hit.Time);

		if (IsSwimming()) //just entered water
		{
			remainingTime += subTimeTickRemaining;
			StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
			return;
		}
		else if (Hit.bBlockingHit)
		{
			if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
			{
				remainingTime += subTimeTickRemaining;
				ProcessLanded(Hit, remainingTime, Iterations);
				return;
			}
			else
			{
				// Compute impact deflection based on final velocity, not integration step.
				// This allows us to compute a new velocity from the deflected vector, and ensures the full gravity effect is included in the slide result.
				Adjusted = Velocity * timeTick;

				// See if we can convert a normally invalid landing spot (based on the hit result) to a usable one.
				if (!Hit.bStartPenetrating && ShouldCheckForValidLandingSpot(timeTick, Adjusted, Hit))
				{
					const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
					FFindFloorResult FloorResult;
					FindFloor(PawnLocation, FloorResult, false);
					if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(PawnLocation, FloorResult.HitResult))
					{
						remainingTime += subTimeTickRemaining;
						ProcessLanded(FloorResult.HitResult, remainingTime, Iterations);
						return;
					}
				}

				HandleImpact(Hit, LastMoveTimeSlice, Adjusted);

				// If we've changed physics mode, abort.
				if (!HasValidData() || !IsFalling())
				{
					return;
				}

				// Limit air control based on what we hit.
				// We moved to the impact point using air control, but may want to deflect from there based on a limited air control acceleration.
				if (bHasAirControl)
				{
					const bool bCheckLandingSpot = false; // we already checked above.
					const FVector AirControlDeltaV = LimitAirControl(LastMoveTimeSlice, AirControlAccel, Hit, bCheckLandingSpot) * LastMoveTimeSlice;
					Adjusted = (VelocityNoAirControl + AirControlDeltaV) * LastMoveTimeSlice;
				}

				const FVector OldHitNormal = Hit.Normal;
				const FVector OldHitImpactNormal = Hit.ImpactNormal;
				FVector Delta = ComputeSlideVector(Adjusted, 1.f - Hit.Time, OldHitNormal, Hit);

				// Compute velocity after deflection (only gravity component for RootMotion)
				if (subTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
				{
					const FVector NewVelocity = (Delta / subTimeTickRemaining);
					Velocity = HasRootMotion() ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
				}

				if (subTimeTickRemaining > KINDA_SMALL_NUMBER && (Delta | Adjusted) > 0.f)
				{
					// Move in deflected direction.
					SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);

					if (Hit.bBlockingHit)
					{
						// hit second wall
						LastMoveTimeSlice = subTimeTickRemaining;
						subTimeTickRemaining = subTimeTickRemaining * (1.f - Hit.Time);

						if (IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit))
						{
							remainingTime += subTimeTickRemaining;
							ProcessLanded(Hit, remainingTime, Iterations);
							return;
						}

						HandleImpact(Hit, LastMoveTimeSlice, Delta);

						// If we've changed physics mode, abort.
						if (!HasValidData() || !IsFalling())
						{
							return;
						}

						// Act as if there was no air control on the last move when computing new deflection.
						if (bHasAirControl && Hit.Normal.Z > 69.0f) // VERTICAL_SLOPE_NORMAL_Z)
						{
							const FVector LastMoveNoAirControl = VelocityNoAirControl * LastMoveTimeSlice;
							Delta = ComputeSlideVector(LastMoveNoAirControl, 1.f, OldHitNormal, Hit);
						}

						FVector PreTwoWallDelta = Delta;
						TwoWallAdjust(Delta, Hit, OldHitNormal);

						// Limit air control, but allow a slide along the second wall.
						if (bHasAirControl)
						{
							const bool bCheckLandingSpot = false; // we already checked above.
							const FVector AirControlDeltaV = LimitAirControl(subTimeTickRemaining, AirControlAccel, Hit, bCheckLandingSpot) * subTimeTickRemaining;

							// Only allow if not back in to first wall
							if (FVector::DotProduct(AirControlDeltaV, OldHitNormal) > 0.f)
							{
								Delta += (AirControlDeltaV * subTimeTickRemaining);
							}
						}

						// Compute velocity after deflection (only gravity component for RootMotion)
						if (subTimeTickRemaining > KINDA_SMALL_NUMBER && !bJustTeleported)
						{
							const FVector NewVelocity = (Delta / subTimeTickRemaining);
							Velocity = HasRootMotion() ? FVector(Velocity.X, Velocity.Y, NewVelocity.Z) : NewVelocity;
						}

						// bDitch=true means that pawn is straddling two slopes, neither of which he can stand on
						bool bDitch = ((OldHitImpactNormal.Z > 0.f) && (Hit.ImpactNormal.Z > 0.f) && (FMath::Abs(Delta.Z) <= KINDA_SMALL_NUMBER) && ((Hit.ImpactNormal | OldHitImpactNormal) < 0.f));
						SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
						if (Hit.Time == 0)
						{
							// if we are stuck then try to side step
							FVector SideDelta = (OldHitNormal + Hit.ImpactNormal).GetSafeNormal2D();
							if (SideDelta.IsNearlyZero())
							{
								SideDelta = FVector(OldHitNormal.Y, -OldHitNormal.X, 0).GetSafeNormal();
							}
							SafeMoveUpdatedComponent(SideDelta, PawnRotation, true, Hit);
						}

						if (bDitch || IsValidLandingSpot(UpdatedComponent->GetComponentLocation(), Hit) || Hit.Time == 0)
						{
							remainingTime = 0.f;
							ProcessLanded(Hit, remainingTime, Iterations);
							return;
						}
						else if (GetPerchRadiusThreshold() > 0.f && Hit.Time == 1.f && OldHitImpactNormal.Z >= GetWalkableFloorZ())
						{
							// We might be in a virtual 'ditch' within our perch radius. This is rare.
							const FVector PawnLocation = UpdatedComponent->GetComponentLocation();
							const float ZMovedDist = FMath::Abs(PawnLocation.Z - OldLocation.Z);
							const float MovedDist2DSq = (PawnLocation - OldLocation).SizeSquared2D();
							if (ZMovedDist <= 0.2f * timeTick && MovedDist2DSq <= 4.f * timeTick)
							{
								Velocity.X += 0.25f * GetMaxSpeed() * (FMath::FRand() - 0.5f);
								Velocity.Y += 0.25f * GetMaxSpeed() * (FMath::FRand() - 0.5f);
								Velocity.Z = FMath::Max<float>(JumpZVelocity * 0.25f, 1.f);
								Delta = Velocity * timeTick;
								SafeMoveUpdatedComponent(Delta, PawnRotation, true, Hit);
							}
						}
					}
				}
			}
		}

		if (Velocity.SizeSquared2D() <= KINDA_SMALL_NUMBER * 10.f)
		{
			Velocity.X = 0.f;
			Velocity.Y = 0.f;
		}
	}
}