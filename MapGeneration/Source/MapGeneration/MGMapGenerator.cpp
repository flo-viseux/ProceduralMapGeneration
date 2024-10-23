#include "MGMapGenerator.h"
#include "CompGeom/Delaunay2.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AMGMapGenerator::AMGMapGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMGMapGenerator::DestroyMap()
{
    TArray<AActor*> CurrentMapPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMGPointActor::StaticClass(), CurrentMapPoints);
    for(int i = CurrentMapPoints.Num() - 1; i >= 0; i--)
    {
        if(CurrentMapPoints[i])
        {
            CurrentMapPoints[i]->Destroy();
        }
    }
}

void AMGMapGenerator::GenerateMap()
{
    DestroyMap();

    // Generate random points
    TArray<FVector2D> Points;
    Points.SetNum(NumPoints);
    for (int32 i = 0; i < NumPoints; i++)
    {
        Points[i] = FVector2D(
            FMath::FRandRange(-MapSize, MapSize),
            FMath::FRandRange(-MapSize, MapSize)
        );
    }

    // Generate Voronoi cells
    TArray<TArray<FVector2D>> VoronoiCells;
    GenerateVoronoiDiagram(Points, VoronoiCells);

    // Calculate MST
    TArray<FEdgeData> MSTEdges;
    CalculateMinimumSpanningTree(Points, MSTEdges);

    // Spawn map elements
    SpawnRoomsAndPaths(Points, MSTEdges);
}

void AMGMapGenerator::GenerateVoronoiDiagram(TArray<FVector2D>& Points, TArray<TArray<FVector2D>>& VoronoiCells)
{
    UE::Geometry::FDelaunay2 Delaunay;
    Delaunay.bAutomaticallyFixEdgesToDuplicateVertices = true;
    bool bSuccess = Delaunay.Triangulate(Points);
    
    if (bSuccess)
    {
        UE::Geometry::FAxisAlignedBox2d Bounds;
        Bounds.Min = FVector2D(-MapSize, -MapSize);
        Bounds.Max = FVector2D(MapSize, MapSize);
        VoronoiCells = Delaunay.GetVoronoiCells(Points, true, UE::Geometry::FAxisAlignedBox2d(Bounds), 0);
    }
}

void AMGMapGenerator::CalculateMinimumSpanningTree(const TArray<FVector2D>& Points, TArray<FEdgeData>& MSTEdges)
{
    TArray<bool> Visited;
    Visited.Init(false, Points.Num());
    
    // Priority queue for Prim's algorithm
    TArray<FEdgeData> PriorityQueue;
    float TotalMSTWeight = 0.0f;
    
    // Start with first vertex
    Visited[0] = true;
    
    // Add all edges from first vertex
    for (int32 i = 1; i < Points.Num(); i++)
    {
        float Distance = FVector2D::Distance(Points[0], Points[i]);
        PriorityQueue.Add(FEdgeData(0, i, Distance)); 
        UE_LOG(LogTemp, Log, TEXT("Add Edge (0, %d) with weight %f"), i, Distance);
    }
    
    // Prim's algorithm
    while (PriorityQueue.Num() > 0)
    {
        // Find minimum weight edge
        float MinWeight = MAX_FLT;
        int32 MinIndex = -1;
        
        for (int32 i = 0; i < PriorityQueue.Num(); i++)
        {
            if (PriorityQueue[i].Weight < MinWeight)
            {
                MinWeight = PriorityQueue[i].Weight;
                MinIndex = i;
            }
        }
        
        if (MinIndex == -1) break;
        
        FEdgeData CurrentEdge = PriorityQueue[MinIndex];
        PriorityQueue.RemoveAt(MinIndex);
        
        // Check if it create cycle  
        if (Visited[CurrentEdge.VertexB])
        {
            UE_LOG(LogTemp, Log, TEXT("Point (%d, %d) ignored because it create a cycle"), CurrentEdge.VertexA, CurrentEdge.VertexB);
            continue;
        }
        
        // Add edge to MST
        MSTEdges.Add(CurrentEdge);
        TotalMSTWeight += CurrentEdge.Weight;
        Visited[CurrentEdge.VertexB] = true;
        
        // Add new edges to priority queue
        for (int32 i = 0; i < Points.Num(); i++)
        {
            if (!Visited[i])
            {
                float Distance = FVector2D::Distance(Points[CurrentEdge.VertexB], Points[i]);
                PriorityQueue.Add(FEdgeData(CurrentEdge.VertexB, i, Distance)); 
                UE_LOG(LogTemp, Log, TEXT("Add Edge (%d, %d) with weight %f"), CurrentEdge.VertexB, i, Distance);
            }
        }

        UE_LOG(LogTemp, Log, TEXT("MST ended with %d edge and weight %f"), MSTEdges.Num(), TotalMSTWeight);
    }
}

void AMGMapGenerator::SpawnRoomsAndPaths(const TArray<FVector2D>& Points, const TArray<FEdgeData>& MSTEdges)
{
    // Spawn rooms at Voronoi vertices
    for (const FVector2D& Point : Points)
    {
        if (PlatformActorClass)
        {
            FVector Location(Point.X, Point.Y, 0.0f);
            FRotator Rotation(0.0f, 0.0f, 0.0f);
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            
            AActor* Room = GetWorld()->SpawnActor<AActor>(PlatformActorClass, Location, Rotation, SpawnParams);
            if (Room)
            {
                Room->SetActorScale3D(FVector(RoomRadius / 50.0f)); // Assuming default cube size is 100
                Room->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
            }
        }
    }

    // Spawn paths between connected rooms
    for (const FEdgeData& Edge : MSTEdges)
    {
        if (PathActorClass)
        {
            FVector Start(Points[Edge.VertexA].X, Points[Edge.VertexA].Y, 0.0f);
            FVector End(Points[Edge.VertexB].X, Points[Edge.VertexB].Y, 0.0f);
            
            FVector Direction = End - Start;
            float Distance = Direction.Size();
            Direction.Normalize();
            
            FVector Location = Start + Direction * (Distance / 2.0f);
            FRotator Rotation = Direction.Rotation();
            
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            
            AActor* Path = GetWorld()->SpawnActor<AActor>(PathActorClass, Location, Rotation, SpawnParams);
            if (Path)
            {
                Path->SetActorScale3D(FVector(Distance / 100.0f, PathWidth / 100.0f, 1.0f));
                Path->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
            }
        }
    }
}

