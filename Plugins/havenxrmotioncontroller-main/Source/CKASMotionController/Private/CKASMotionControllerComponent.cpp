// Copyright Epic Games, Inc. All Rights Reserved.

#include "CKASMotionControllerComponent.h"
#include "CKASMotionControllerUtils.h"
#include "GameFramework/Actor.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "MovieSceneSequencePlayer.h"
#include "Engine/World.h"

UCKASMotionControllerComponent::UCKASMotionControllerComponent() : 
	LocationAverage(CreateDefaultSubobject<UMovingAverage>(TEXT("LocationAverage"))), 
	VelocityAverage(nullptr),
	AccelerationAverage(nullptr),
	AngularVelocityAverage(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	bAllowConcurrentTick = true;
}

UCKASMotionControllerComponent::~UCKASMotionControllerComponent()
{
	
}

void UCKASMotionControllerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (LocationAverage)
	{
		LocationAverage->Initialize(LocationWindowSize);
	}

	if (!AccelerationAverage)
	{
		AccelerationAverage = NewObject<UMovingAverage>(this);
		AccelerationAverage->Initialize(LocationWindowSize);
	}

	if (!AngularVelocityAverage)
	{
		AngularVelocityAverage = NewObject<UMovingAverage>(this);
		AngularVelocityAverage->Initialize(LocationWindowSize);
	}
}

FVector UCKASMotionControllerComponent::CalculateInertialAcceleration(const USceneComponent* Owner, const float FrameDeltaSeconds, const FVector* InertialAccelerationMultiplier)
{
    const FVector CurrentLocation = Owner->GetComponentLocation();

    // Calculate velocity directly
    const FVector CurrentVelocity = Owner->GetComponentTransform().InverseTransformVectorNoScale(CurrentLocation - PreviousLocation) / FrameDeltaSeconds;
    PreviousLocation = CurrentLocation;

    // Calculate acceleration directly
    const FVector CurrentAcceleration = (CurrentVelocity - PreviousVelocity) / FrameDeltaSeconds;
    PreviousVelocity = CurrentVelocity;

    // Apply multiplier and round components to 2 decimal places
    return FVector(
        CurrentAcceleration.X / 100 * InertialAccelerationMultiplier->X,
        CurrentAcceleration.Y / 100 * InertialAccelerationMultiplier->Y,
        CurrentAcceleration.Z / 100 * InertialAccelerationMultiplier->Z
    );
}

FVector UCKASMotionControllerComponent::CalculateGravityProjection(const USceneComponent* Owner) const
{
	const FVector GravityProjection = Owner->GetComponentTransform().TransformVector(BaseGravity);
	return GravityProjection;
}

FVector UCKASMotionControllerComponent::CalculateAngularVelocity(const USceneComponent* Owner, const float FrameDeltaSeconds, const FVector* AngularVelocityMultiplier)
{
	const FQuat CurrentQuat = Owner->GetComponentQuat();

	if (PreviousQuat.IsIdentity())
	{
		PreviousQuat = CurrentQuat;
		return FVector::ZeroVector;
	}

	// Calculate delta quaternion
	const FQuat DeltaQuat = CurrentQuat * PreviousQuat.Inverse();
	FVector Axis;
	float Angle;
	DeltaQuat.ToAxisAndAngle(Axis, Angle);

	// Ensure smallest angle (handle wraparound)
	if (Angle > PI)
	{
		Angle -= 2.0f * PI;
	}

	// Angular velocity (rad/s)
	const FVector AngularVelocity = (Axis * Angle) / FrameDeltaSeconds;

	PreviousQuat = CurrentQuat;

	return FVector(
		AngularVelocity.X * AngularVelocityMultiplier->X,
		AngularVelocity.Y * AngularVelocityMultiplier->Y,
		AngularVelocity.Z * AngularVelocityMultiplier->Z
	);
}

void UCKASMotionControllerComponent::GetPlatformPhysics(
    const USceneComponent* Owner,
    const FVector& InertialAccelerationMultiplier,
    const FVector& AngularVelocityMultiplier,
    FVector& InertialAcceleration,
    FVector& GravityProjection,
    FVector& AngularVelocity,
    const float DeltaTime)
{
    

    // Calculate raw values
    const FVector RawInertialAcceleration = CalculateInertialAcceleration(Owner, DeltaTime, &InertialAccelerationMultiplier);
    const FVector RawAngularVelocity = CalculateAngularVelocity(Owner, DeltaTime, &AngularVelocityMultiplier);
    GravityProjection = CalculateGravityProjection(Owner);

    // Define thresholds
    constexpr float InertialAccelerationThreshold = 50.0f;
    constexpr float AngularVelocityThreshold = 50.0f;

    // Reject large frames for InertialAcceleration
    if (RawInertialAcceleration.Size() > InertialAccelerationThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Rejected RawInertialAcceleration: %s"), *RawInertialAcceleration.ToString());
        //InertialAcceleration = FVector::ZeroVector;
    }
    else if (AccelerationAverage)
    {
        AccelerationAverage->AddSample(RawInertialAcceleration);
        InertialAcceleration = AccelerationAverage->GetAverage();
    } 
    else
    {
        InertialAcceleration = RawInertialAcceleration;
    }

    // Reject large frames for AngularVelocity
    if (RawAngularVelocity.Size() > AngularVelocityThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Rejected RawAngularVelocity: %s"), *RawAngularVelocity.ToString());
        //AngularVelocity = FVector::ZeroVector;
    }
    else if (AngularVelocityAverage)
    {
        AngularVelocityAverage->AddSample(RawAngularVelocity);
        AngularVelocity = AngularVelocityAverage->GetAverage();
    } 
    else
    {
        AngularVelocity = RawAngularVelocity;
    }
}

FString UCKASMotionControllerComponent::VectorToString(const FVector& Vector, const bool IsXZFlipped)
{
	return IsXZFlipped
		       ? FString::Printf(TEXT("%.3f %.3f %.3f"), Vector.Z, Vector.Y, Vector.X)
		       : FString::Printf(TEXT("%.3f %.3f %.3f"), Vector.X, Vector.Y, Vector.Z);
}

FString UCKASMotionControllerComponent::RotatorToString(const FRotator& Rotator)
{
	return FString::Printf(TEXT("%.3f %.3f %.3f"), Rotator.Yaw, Rotator.Roll, Rotator.Pitch);
}

FString UCKASMotionControllerComponent::CreateCKASString(
	const FVector& Location,
	const FRotator& Rotation,
	const FVector& InertialAcceleration,
	const FVector& GravityProjection,
	const FVector& AngularVelocity)
{
	FString CKASString;

	return CKASString += TEXT("~M ")
		+ VectorToString(Location, false) + TEXT(" ")
		+ RotatorToString(Rotation) + TEXT(" ")
		+ VectorToString(InertialAcceleration, false) + TEXT(" ")
		+ VectorToString(GravityProjection, false) + TEXT(" ")
		+ VectorToString(AngularVelocity, true);
}