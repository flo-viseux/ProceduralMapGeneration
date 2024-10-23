#pragma once

#include "CoreMinimal.h"
#include "FEdgeData.generated.h"

USTRUCT(BlueprintType)
struct FEdgeData
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY()
    int32 VertexA;

    UPROPERTY()
    int32 VertexB;

    UPROPERTY()
    float Weight;

    FEdgeData() : VertexA(0), VertexB(0), Weight(0.0f) {}

    FEdgeData(int32 InVertexA, int32 InVertexB, float InWeight)
        : VertexA(InVertexA), VertexB(InVertexB), Weight(InWeight)
    {}

    bool operator<(const FEdgeData& Other) const
    {
        return Weight < Other.Weight;
    }
};