#include "CKASMotionControllerUtils.h"

UMovingAverage::UMovingAverage()
    : WindowSize(1)
{
}

void UMovingAverage::Initialize(int32 InWindowSize)
{
    WindowSize = InWindowSize;
    Samples.Empty();
}

void UMovingAverage::AddSample(const FVector& Sample)
{
    if (Samples.Num() >= WindowSize)
    {
        Samples.RemoveAt(0);
    }
    Samples.Add(Sample);
}

FVector UMovingAverage::GetAverage() const
{
    if (Samples.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector Sum = FVector::ZeroVector;
    for (const FVector& Sample : Samples)
    {
        Sum += Sample;
    }
    return Sum / Samples.Num();
}