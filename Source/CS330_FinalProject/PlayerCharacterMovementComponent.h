// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "Engine.h"
#include "PlayerCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class CS330_FINALPROJECT_API UPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	public:
		UPlayerCharacterMovementComponent();
		
		virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_AirAccelerationFriction;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_AirAcceleration;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_GroundAcceleration;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_AirAccelerationMaxVelocity;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_GroundAccelerationMaxVelocity;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_MinInitialVelocity;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_HopImpulse;
    
    
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_DeltaTimeForPhysics;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_InitialAirAccelerationFriction;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_InitialAirAcceleration;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_InitialGroundAcceleration;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_InitialAirAccelerationMaxVelocity;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_InitialGroundAccelerationMaxVelocity;
        UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") float C_InitialMinInitialVelocity;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") FVector AAccel;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") FVector AVel;
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Airstrafe") bool dbgArbitraryAccel;

		void StartJumping();
		void StopJumping();
		bool AutoHop;

		double GetCurrentSpeed();

private:
	void GetHit(FHitResult& Hit, bool& HaveContact, float DeltaTime);
	
	FVector Accelerate(FVector accelDir, FVector prevVelocity, float accelerate, float max_velocity, float DeltaTime);
	FVector MoveGround(FVector accelDir, FVector prevVelocity, float DeltaTime);
	FVector MoveAir(FVector accelDir, FVector prevVelocity, float DeltaTime);

protected:
	// override this for ramp behavior
	virtual FVector HandleSlopeBoosting(const FVector& SlideResult, const FVector& Delta, const float Time, const FVector& Normal, const FHitResult& Hit) const;

};
