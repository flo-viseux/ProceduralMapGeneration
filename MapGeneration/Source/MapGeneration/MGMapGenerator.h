// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGPointActor.h"
#include "FEdgeData.h"
#include "GameFramework/Actor.h"
#include "MGMapGenerator.generated.h"

UCLASS()
class MAPGENERATION_API AMGMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AMGMapGenerator();

	UFUNCTION(CallInEditor, Category = "Generation")
    void GenerateMap();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 NumPoints = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float MapSize = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float RoomRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float PathWidth = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    TSubclassOf<AMGPointActor> PlatformActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    TSubclassOf<AMGPointActor> PathActorClass;
    
private:
    void DestroyMap();
    void GenerateVoronoiDiagram(TArray<FVector2D>& Points, TArray<TArray<FVector2D>>& VoronoiCells);
    void CalculateMinimumSpanningTree(const TArray<FVector2D>& Points, TArray<FEdgeData>& MSTEdges);
    void SpawnRoomsAndPaths(const TArray<FVector2D>& Points, const TArray<FEdgeData>& MSTEdges);

};
