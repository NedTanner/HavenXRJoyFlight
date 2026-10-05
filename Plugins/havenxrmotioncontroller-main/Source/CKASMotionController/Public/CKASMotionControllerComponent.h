// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CKASMotionControllerUtils.h"
#include "LevelSequenceActor.h"
#include "MovieSceneSequencePlayer.h"
#include "CKASMotionControllerComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSequenceUpdated, const UMovieSceneSequencePlayer&, FFrameTime, FFrameTime);

/**
 *	CKASMotionControllerComponent is a component that provides the ability to interface with a CKAS motion platform.
 *	This component is responsible for calculating the physics properties of the platform and formatting the data to be
 *	sent to SIMCOR DX via UDP socket connection.
 *
 *	The component also provides the ability to smooth the data using moving averages.
 *	Intended to be used with an AActor that represents the platform, and to be extended by
 *	the BP_CKASMotionControllerComponent.
 */

/**
 * TODO: Investigate adding mode to update position on sequencer frame.
 */
UCLASS(Abstract, ClassGroup=(Custom), Blueprintable, BlueprintType)
class CKASMOTIONCONTROLLER_API UCKASMotionControllerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCKASMotionControllerComponent();
    virtual ~UCKASMotionControllerComponent() override;

protected:
    virtual void BeginPlay() override;
    
    /**
     * Calculates and retrieves the platform's physics properties.
     *
     * @param Owner The USceneComponent representing the platform whose physics properties are to be calculated.
     * @param InertialAccelerationMultiplier Strength of InertialAcceleration.
     * @param AngularVelocityMultiplier Strength of AngularVelocity.
     * @param InertialAcceleration The acceleration of the platform due to inertia.
     * @param GravityProjection The projection of the base gravity vector in the platform's local space.
     * @param AngularVelocity The angular velocity of the platform.
     * @param FrameDeltaSeconds
     */
    UFUNCTION(BlueprintCallable, Category = "CKAS Motion Platform")
    void GetPlatformPhysics(const USceneComponent* Owner,
                            const FVector& InertialAccelerationMultiplier,
                            const FVector& AngularVelocityMultiplier,
                            FVector& InertialAcceleration,
                            FVector& GravityProjection,
                            FVector& AngularVelocity,
                            const float DeltaTime);
    
    /**
     * Creates a formatted string representing the CKAS platform state.
     * 
     * @param Location The current location of the platform as a FVector.
     * @param Rotation The current rotation of the platform as a FRotator.
     * @param InertialAcceleration The inertial acceleration of the platform as a FVector.
     * @param GravityProjection The projection of gravity on the platform as a FVector.
     * @param AngularVelocity The angular velocity of the platform as a FVector.
     * @return FString A formatted string representing the platform's state.
     */
    UFUNCTION(BlueprintPure, Category = "CKAS Motion Platform")
    static FString CreateCKASString(const FVector& Location, const FRotator& Rotation,
                                    const FVector& InertialAcceleration, const FVector& GravityProjection,
                                    const FVector& AngularVelocity);

private:
    /** Calculate Interial Acceleration of owner */
    FVector CalculateInertialAcceleration(const USceneComponent* Owner, const float FrameDeltaSeconds, const FVector* InertialAccelerationMultiplier);

    /** Calculate Gravity Projection of owner */
    FVector CalculateGravityProjection(const USceneComponent* Owner) const;

    /** Calculate Angular Velocity of owner */
    FVector CalculateAngularVelocity(const USceneComponent* Owner, const float FrameDeltaSeconds, const FVector* AngularVelocityMultiplier);

    /** Convert vectors to string */
    static FString VectorToString(const FVector& Vector, bool IsXZFlipped);

    /** Convert rotators to string */
    static FString RotatorToString(const FRotator& Rotator);
    
    /** The previous location, velocity, rotation, and angular velocity of the platform */
    FVector PreviousLocation { FVector::ZeroVector };
    FVector PreviousVelocity { FVector::ZeroVector };;
    FRotator PreviousRotation { FRotator::ZeroRotator };
    FVector PreviousAngularVelocity { FVector::ZeroVector };;
    FVector BaseGravity { FVector(0.0f, 0.0f, 9.81f) };
    FQuat PreviousQuat { FQuat::Identity };
    
    // Reference to the Level Sequence Actor
    UPROPERTY()
    TObjectPtr<ALevelSequenceActor> SequenceActor;
    
    /** Define the window size for the moving average */
    UPROPERTY()
    int32 LocationWindowSize { 60 };

    /** Moving average calculators for location smoothing */
    UPROPERTY()
    TObjectPtr<UMovingAverage> LocationAverage;

    /** Moving average calculator for velocity smoothing */
    UPROPERTY()
    TObjectPtr<UMovingAverage> VelocityAverage;

    /** Moving average calculator for acceleration smoothing */
    UPROPERTY()
    TObjectPtr<UMovingAverage> AccelerationAverage;

    /** Moving average calculator for angular velocity smoothing */
    UPROPERTY()
    TObjectPtr<UMovingAverage> AngularVelocityAverage;
};



