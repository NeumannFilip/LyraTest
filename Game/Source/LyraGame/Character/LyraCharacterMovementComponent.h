// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/HitResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathSSE.h"
#include "NativeGameplayTags.h"
#include "UObject/UObjectGlobals.h"

#include "LyraCharacterMovementComponent.generated.h"

class UObject;
class ALyraCharacter;
struct FFrame;

LYRAGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_MovementStopped);

USTRUCT(BlueprintType)
struct FLyraCharacterGroundInfo
{
	GENERATED_BODY()

	FLyraCharacterGroundInfo()
		: LastUpdateFrame(0)
		, GroundDistance(0.0f)
	{}

	uint64 LastUpdateFrame;

	UPROPERTY(BlueprintReadOnly)
	FHitResult GroundHitResult;

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance;
};

UCLASS(Config = Game)
class LYRAGAME_API ULyraCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	ULyraCharacterMovementComponent();
	ULyraCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void SimulateMovement(float DeltaTime) override;


	// Returns the current ground info.  Calling this will update the ground info if it's out of date.
	UFUNCTION(BlueprintCallable, Category = "Lyra|CharacterMovement")
	const FLyraCharacterGroundInfo& GetGroundInfo();

	void SetReplicatedAcceleration(const FVector& InAcceleration);

	//~UMovementComponent interface
	virtual FRotator GetDeltaRotation(float DeltaTime) const override;
	//virtual float GetMaxSpeed() const override;
	//~End of UMovementComponent inter



	////////////////////////////
	///Bhop / Surf
	//////////////////////

	// Sector 1 : Overridable classes
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual bool DoJump(bool bReplayingMoves) override;
	
	virtual bool CanAttemptJump() const override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual void ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration) override;
	virtual bool ShouldLimitAirControl(float DeltaTime, const FVector& FallAcceleration) const override;
	void ApplyAcceleration(float DeltaTime, float SurfaceFriction, const FVector& WishDirection, float WishSpeed, float Acceleration);
	
	// MovementModes: Walking, Sprinting, Crouching?
	void SetSlowWalking(bool bNewIsSlowWalking);


	virtual float GetMaxSpeed() const override;
	
	// Sector 2 : custom methods


	// Sector 3 : Variables
	//Timestamp when character was in-air
	float LastAirTimestamp;
	UPROPERTY(Category = " Movement: Bunnyhopping", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float NoFrictionAfterLandingTime;

	// walking
	UPROPERTY(Category = " Movement: Walking", EditAnywhere, BlueprintReadWrite)
	uint32 bCanSlowWalk : 1;
	//Define if character is walking
	UPROPERTY(Category = "Movement: Walking", VisibleAnywhere, BlueprintReadOnly)
	uint32 bIsSlowWalking : 1;

	UPROPERTY(Category = " Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bCanSlowWalk", ClampMin = "0", UIMin = "0", ClampMax = "1", UIMax = "1"))
	float SlowWalkingMaxSpeedMultiplier;
	
	uint32 bUseEnforcedMaxSpeed;

	UPROPERTY(Category = " Movement: Jumping / Falling", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxFallAirSpeed;
	
	UPROPERTY(Category = "Movement: Bunnyhopping", EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bUseEnforcedMaxSpeed", ClampMin = "0", UIMin = "0"))
	float EnforcedMaxSpeed;
	
	// Acceleration & Friction
	void ApplyAirAcceleration(float DeltaTime, float SurfaceFriction, const FVector& WishDirection, float WishSpeed, float MaxAirWishSpeed, float Acceleration);
	void ApplyFriction(float DeltaTime, float CharacterFriction, float SurfaceFriction, float StopSpeed);
	bool ShouldApplyGroundFriction();

	UPROPERTY(Category = "Movement: Jumping", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float MaxAirAcceleration;
	UPROPERTY(Category = "Movement: Walking", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float StopSpeed;

	
	float GetMaxAcceleration() const override;
	// braking
	

protected:

	virtual void InitializeComponent() override;
	// Override to handle resetting jump parameters upon landing
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	// Cached ground info for the character.  Do not access this directly!  It's only updated when accessed via GetGroundInfo().
	FLyraCharacterGroundInfo CachedGroundInfo;
	UPROPERTY(Transient)
	bool bHasReplicatedAcceleration = false;

	///Mine 

	
};


