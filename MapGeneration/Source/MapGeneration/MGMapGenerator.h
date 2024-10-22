// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGPointActor.h"
#include "GameFramework/Actor.h"
#include "MGMapGenerator.generated.h"

UCLASS()
class MAPGENERATION_API AMGMapGenerator : public AActor
{
	GENERATED_BODY()

public:
	AMGMapGenerator();

	UFUNCTION(CallInEditor, Category = "MapGeneration")
	void GenerateMap();

private:
	UPROPERTY(EditAnywhere, Category = "MapGeneration", meta = (ClampMin = "10", ClampMax = "1000"))
	int32 NumVertices = 100;

	UPROPERTY(EditAnywhere, Category = "MapGeneration", meta = (ClampMin = "100", ClampMax = "10000"))
	float MapSize = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "MapGeneration", meta = (ClampMin = "10", ClampMax = "400"))
	float PlatformRadius = 50.0f;

	UPROPERTY(EditAnywhere, Category = "MapGeneration", meta = (ClampMin = "1", ClampMax = "200"))
	float PathWidth = 10.0f;

	UPROPERTY(EditAnywhere, Category = "MapGeneration")
	TSubclassOf<AMGPointActor> PlatformActorClass;

	UPROPERTY(EditAnywhere, Category = "MapGeneration")
	TSubclassOf<AMGPointActor> PathActorClass;

	void DestroyMap();

	void GenerateDelaunayTriangulation(TArray<FVector2D>& OutVertices, TArray<TPair<int32, int32>>& OutEdges);
	
	void GenerateMinimumSpanningTree(const TArray<FVector2D>& Vertices, const TArray<TPair<int32, int32>>& Edges, TArray<int32>& OutTreeEdges);
	
	void SpawnMapGeometry(const TArray<FVector2D>& Vertices, const TArray<int32>& TreeEdges);

	void SpawnPlatform(const FVector2D& Location);
	void SpawnPath(const FVector2D& Start, const FVector2D& End);

};
