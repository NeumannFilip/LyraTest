// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "CharacterMovementComponentAsync.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Containers/EnumAsByte.h"
#include "CoreGlobals.h"
#include "LyraCharacter.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "HAL/IConsoleManager.h"
#include "Math/Vector.h"
#include "Misc/AssertionMacros.h"
#include "NativeGameplayTags.h"
#include "Stats/Stats2.h"
#include "UObject/NameTypes.h"
#include "UObject/ObjectPtr.h"
//extra includes
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/PhysicsSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraCharacterMovementComponent)

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_MovementStopped, "Gameplay.MovementStopped");

// Taken from CharacterMovementComponent.cpp
// Version that does not use inverse sqrt estimate, for higher precision.
FORCEINLINE FVector GetSafeNormalPrecise(const FVector& V)
{
	const auto VSq = V.SizeSquared();
	if (VSq < SMALL_NUMBER)
	{
		return FVector::ZeroVector;
	}
	else
	{
		return V * (1.f / FMath::Sqrt(VSq));
	}
}

//Gravity 
constexpr float DesiredGravity = -1143.0f;

namespace LyraCharacter
{
	static float GroundTraceDistance = 100000.0f;
	FAutoConsoleVariableRef CVar_GroundTraceDistance(TEXT("LyraCharacter.GroundTraceDistance"), GroundTraceDistance, TEXT("Distance to trace down when generating ground information."), ECVF_Cheat);
};


ULyraCharacterMovementComponent::ULyraCharacterMovementComponent()
{
	//Variables crucial for adjusting behaviour of movement
	//First of all Friction and Braking
	GroundFriction = 4.0f;
	BrakingFriction = 4.0f; 
	bUseSeparateBrakingFriction = false;
	BrakingFrictionFactor = 1.0f;
	FallingLateralFriction = 0.0f;
	BrakingDecelerationFalling = 0.0f;
	BrakingDecelerationFlying = 190.5f;

	//Air
	AirControl = 1.0f;
	AirControlBoostMultiplier = 1.0f;
	AirControlBoostVelocityThreshold = 0.0f;
	
	//Jump
	JumpZVelocity = 400.0f;
	
	//Physics
	StandingDownwardForceScale = 1.0f;
	GravityScale = DesiredGravity / UPhysicsSettings::Get()->DefaultGravityZ;
	InitialPushForceFactor = 100.0f;
	PushForceFactor = 500.0f;
	
	
	// Push Force
	bPushForceUsingZOffset = false;
	PushForcePointZOffsetFactor = -0.66f;

	//Speed
	
	MaxWalkSpeed = 700.0f;
	StopSpeed = 150.0f;
	MaxFallAirSpeed = 100.0f;
	
	//Acceleration
	MaxAcceleration = 900.0f;
	

	/// Angle & Slope
	SetWalkableFloorZ(0.7f);

	//Character
	bUseControllerDesiredRotation = false;
	bUseFlatBaseForFloorChecks = true;
	CrouchedHalfHeight = 52.0f;
	
	//Extra commands
	bMaintainHorizontalGroundVelocity = true;
}

//s
ULyraCharacterMovementComponent::ULyraCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void ULyraCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (IsFalling())
	{
		LastAirTimestamp = GetWorld()->GetTimeSeconds();
	}
	
	if (!bCanSlowWalk && bIsSlowWalking)
	{
		bIsSlowWalking = false;
	}
}

bool ULyraCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	const auto bJumped = Super::DoJump(bReplayingMoves);
	return bJumped;
}

void ULyraCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
	float BrakingDeceleration)
{
	Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
	//Usind the modes created in GetMaxSpeed I keep track of them

	//I also need to check if the friction is correct
	Friction = FMath::Max(0.f, Friction);
	const auto MaxAcceleration = GetMaxAcceleration();
	auto MaxSpeed = GetMaxSpeed();
	//Check if pathing is correct

	auto bRequestedZeroAcceleration = true;
	auto RequestedAcceleration = FVector::ZeroVector;
	auto RequestedSpeed = 0.0f;
	if(ApplyRequestedMove(DeltaTime, MaxAcceleration, Friction, MaxSpeed, BrakingDeceleration,RequestedAcceleration, RequestedSpeed))
	{
		RequestedAcceleration = RequestedAcceleration.GetClampedToMaxSize(MaxAcceleration);
		bRequestedZeroAcceleration = false;
	}

	//Friction
	if(ShouldApplyGroundFriction())
	{
		ApplyFriction(DeltaTime, Friction, 1.0f, StopSpeed);
	}

	//Acceleration
	const auto AccelerationDirection = GetSafeNormalPrecise(Acceleration);
	const auto AccelerationAmount = Acceleration.Size();
	
	if(IsMovingOnGround())
	{
		ApplyAcceleration(DeltaTime, 1.0f, AccelerationDirection, MaxSpeed, AccelerationAmount);
	}
	else if(IsFalling())
	{
		//TODO Implement AirAccel
		ApplyAirAcceleration(DeltaTime, 1.0f, AccelerationDirection, MaxSpeed, MaxFallAirSpeed,AccelerationAmount);
	}

	
}

void ULyraCharacterMovementComponent::SimulateMovement(float DeltaTime)
{
	if (bHasReplicatedAcceleration)
	{
		// Preserve our replicated acceleration
		const FVector OriginalAcceleration = Acceleration;
		Super::SimulateMovement(DeltaTime);
		Acceleration = OriginalAcceleration;
	}
	else
	{
		Super::SimulateMovement(DeltaTime);
	}
}

bool ULyraCharacterMovementComponent::CanAttemptJump() const
{
	bool bCanAttemptJump = IsJumpAllowed();
	if (IsMovingOnGround())
	{
		const float FloorZ = FVector(0.0f, 0.0f, 1.0f) | CurrentFloor.HitResult.ImpactNormal;
		const float WalkableFloor = GetWalkableFloorZ();
		bCanAttemptJump &= (FloorZ >= WalkableFloor) || FMath::IsNearlyEqual(FloorZ, WalkableFloor);
	}
	return bCanAttemptJump;
}

void ULyraCharacterMovementComponent::ApplyVelocityBraking(float DeltaTime, float Friction, float BrakingDeceleration)
{
	
	Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
}

bool ULyraCharacterMovementComponent::ShouldLimitAirControl(float DeltaTime, const FVector& FallAcceleration) const
{
	return false;
}

void ULyraCharacterMovementComponent::ApplyAcceleration(float DeltaTime, float SurfaceFriction,
	const FVector& WishDirection, float WishSpeed, float Acceleration)
{
	const auto VelocityProgress = FVector::DotProduct(Velocity, WishDirection);
	const auto AddSpeed = WishSpeed - VelocityProgress;
	if (AddSpeed <= 0.0f)
	{
		return;
	}

	auto AccelerationSpeed = Acceleration * WishSpeed * SurfaceFriction * DeltaTime;
	AccelerationSpeed = FMath::Min(AccelerationSpeed, AddSpeed);

	Velocity += AccelerationSpeed * WishDirection;
}


void ULyraCharacterMovementComponent::SetSlowWalking(bool bNewIsSlowWalking)
{
}


void ULyraCharacterMovementComponent::ApplyAirAcceleration(float DeltaTime, float SurfaceFriction,
	const FVector& WishDirection, float WishSpeed, float MaxAirWishSpeed, float Acceleration)
{

	const auto AirWishSpeed = FMath::Min(WishSpeed, MaxAirWishSpeed);
	const auto VelocityProgress = FVector::DotProduct(Velocity, WishDirection);
	const auto AddSpeed = AirWishSpeed - VelocityProgress;
	if(AddSpeed <= 0.0f)
		return;

	auto AccelerationSpeed = Acceleration * WishSpeed * SurfaceFriction * DeltaTime;

	AccelerationSpeed = FMath::Min(AccelerationSpeed, AddSpeed);

	Velocity += AccelerationSpeed * WishDirection;
}

void ULyraCharacterMovementComponent::ApplyFriction(float DeltaTime, float CharacterFriction, float SurfaceFriction,
	float StopSpeed)
{
	const auto Speed = Velocity.Size();
	auto ControlSpeed = FMath::Max(StopSpeed, Speed);
	auto SpeedDrop = ControlSpeed * CharacterFriction * SurfaceFriction * DeltaTime;

	Velocity *= FMath::Max(0.0f, Speed - SpeedDrop) / Speed;
}

bool ULyraCharacterMovementComponent::ShouldApplyGroundFriction()
{
	//if on ground OR time window expired
	//TODO:: include option for autobhop
	return (IsMovingOnGround() && GetWorld()->TimeSeconds >= LastAirTimestamp + NoFrictionAfterLandingTime);
}

float ULyraCharacterMovementComponent::GetMaxAcceleration() const
{
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
	case MOVE_Falling:
	case MOVE_Flying:
		return MaxAirAcceleration;

	case MOVE_None:
	default:
		return 0.f;
	}
}

void ULyraCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}



void ULyraCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode,
	uint8 PreviousCustomMode)
{
	
	
}

auto ULyraCharacterMovementComponent::GetGroundInfo() -> const FLyraCharacterGroundInfo&
{
	if (!CharacterOwner || (GFrameCounter == CachedGroundInfo.LastUpdateFrame))
    	{
    		return CachedGroundInfo;
    	}
    
    	if (MovementMode == MOVE_Walking)
    	{
    		CachedGroundInfo.GroundHitResult = CurrentFloor.HitResult;
    		CachedGroundInfo.GroundDistance = 0.0f;
    	}
    	else
    	{
    		const UCapsuleComponent* CapsuleComp = CharacterOwner->GetCapsuleComponent();
    		check(CapsuleComp);
    
    		const float CapsuleHalfHeight = CapsuleComp->GetUnscaledCapsuleHalfHeight();
    		const ECollisionChannel CollisionChannel = (UpdatedComponent ? UpdatedComponent->GetCollisionObjectType() : ECC_Pawn);
    		const FVector TraceStart(GetActorLocation());
    		const FVector TraceEnd(TraceStart.X, TraceStart.Y, (TraceStart.Z - LyraCharacter::GroundTraceDistance - CapsuleHalfHeight));
    
    		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LyraCharacterMovementComponent_GetGroundInfo), false, CharacterOwner);
    		FCollisionResponseParams ResponseParam;
    		InitCollisionParams(QueryParams, ResponseParam);
    
    		FHitResult HitResult;
    		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CollisionChannel, QueryParams, ResponseParam);
    
    		CachedGroundInfo.GroundHitResult = HitResult;
    		CachedGroundInfo.GroundDistance = LyraCharacter::GroundTraceDistance;
    
    		if (MovementMode == MOVE_NavWalking)
    		{
    			CachedGroundInfo.GroundDistance = 0.0f;
    		}
    		else if (HitResult.bBlockingHit)
    		{
    			CachedGroundInfo.GroundDistance = FMath::Max((HitResult.Distance - CapsuleHalfHeight), 0.0f);
    		}
    	}
    
    	CachedGroundInfo.LastUpdateFrame = GFrameCounter;
    
    	return CachedGroundInfo;
}

void ULyraCharacterMovementComponent::SetReplicatedAcceleration(const FVector& InAcceleration)
{

}

FRotator ULyraCharacterMovementComponent::GetDeltaRotation(float DeltaTime) const
{

	return Super::GetDeltaRotation(DeltaTime);
}



float ULyraCharacterMovementComponent::GetMaxSpeed() const
{
	auto Speed = 0.0f;
	switch (MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
	case MOVE_Falling:
		Speed = (IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed);
		break;
	case MOVE_Flying:
		Speed = MaxFlySpeed;
		break;
	case MOVE_None:
	default:
		Speed = 0.f;
		break;
	}

	if (bIsSlowWalking)
	{
		Speed *= SlowWalkingMaxSpeedMultiplier;
	}

	return Speed;
}