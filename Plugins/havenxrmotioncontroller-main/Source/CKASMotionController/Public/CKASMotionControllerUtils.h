// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CKASMotionControllerUtils.generated.h"

UCLASS()
class CKASMOTIONCONTROLLER_API UMovingAverage : public UObject
{
    GENERATED_BODY()

public:
    UMovingAverage();

    void Initialize(int32 InWindowSize);
    void AddSample(const FVector& Sample);
    FVector GetAverage() const;

private:
    int32 WindowSize;
    TArray<FVector> Samples;
};