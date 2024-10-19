#include "MapGenerator.h"
#include "CompGeom/Delaunay2.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AMapGenerator::AMapGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AMapGenerator::DestroyMap()
{
    TArray<AActor*> CurrentMapPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APointActor::StaticClass(), CurrentMapPoints);
    for(int i = CurrentMapPoints.Num() - 1; i >= 0; i--)
    {
        if(CurrentMapPoints[i])
        {
            CurrentMapPoints[i]->Destroy();
        }
    }
}

void AMapGenerator::GenerateMap()
{
    DestroyMap();
    
    TArray<FVector2D> Vertices;
    TArray<TPair<int32, int32>> Edges;
    GenerateDelaunayTriangulation(Vertices, Edges);

    TArray<int32> TreeEdges;
    GenerateMinimumSpanningTree(Vertices, Edges, TreeEdges);

    SpawnMapGeometry(Vertices, TreeEdges);
}

void AMapGenerator::GenerateDelaunayTriangulation(TArray<FVector2D>& OutVertices, TArray<TPair<int32, int32>>& OutEdges)
{
    // Generate Random points in map area
    OutVertices.SetNum(NumVertices);
    for (int32 i = 0; i < NumVertices; ++i)
    {
        OutVertices[i] = FVector2D(FMath::RandRange(-MapSize, MapSize), FMath::RandRange(-MapSize, MapSize));
    }

    // Use Delauney Geometry implementation for Delauney triangulation
    UE::Geometry::FDelaunay2 Delaunay;
    bool bSuccess = Delaunay.Triangulate(OutVertices);

    // If Delauney triangulation succeed, add triangles edges in OutEdges array
    if (bSuccess)
    {
        const TArray<UE::Geometry::FIndex3i>& Triangles = Delaunay.GetTriangles();
        for (const auto& Triangle : Triangles)
        {
            OutEdges.Add(TPair<int32, int32>(Triangle.A, Triangle.B));
            OutEdges.Add(TPair<int32, int32>(Triangle.B, Triangle.C));
            OutEdges.Add(TPair<int32, int32>(Triangle.C, Triangle.A));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Delaunay triangulation failed"));
    }
}

void AMapGenerator::GenerateMinimumSpanningTree(const TArray<FVector2D>& Vertices, const TArray<TPair<int32, int32>>& Edges, TArray<int32>& OutTreeEdges)
{
    // Create minimum spanning tree
    TArray<int32> Parent;
    Parent.Init(-1, Vertices.Num());

    auto Find = [&Parent](int32 i) {
        while (Parent[i] != -1) i = Parent[i];
        return i;
    };

    // Sort edges by length
    TArray<TPair<int32, int32>> SortedEdges = Edges;
    SortedEdges.Sort([&Vertices](const TPair<int32, int32>& A, const TPair<int32, int32>& B) {
        float LengthA = FVector2D::DistSquared(Vertices[A.Key], Vertices[A.Value]);
        float LengthB = FVector2D::DistSquared(Vertices[B.Key], Vertices[B.Value]);
        return LengthA < LengthB;
    });

    // Go through the sorted edges and add them to the minimum spanning tree only if they don't create a cycle
    for (const auto& Edge : SortedEdges)
    {
        int32 SetA = Find(Edge.Key);
        int32 SetB = Find(Edge.Value);

        if (SetA != SetB)
        {
            OutTreeEdges.Add(Edge.Key);
            OutTreeEdges.Add(Edge.Value);
            Parent[SetA] = SetB;
        }
    }
}

void AMapGenerator::SpawnMapGeometry(const TArray<FVector2D>& Vertices, const TArray<int32>& TreeEdges)
{
    // Create a platform for all vertices
    for (const FVector2D& Vertex : Vertices)
    {
        SpawnPlatform(Vertex);
    }

    // Create path between platforms
    for (int32 i = 0; i < TreeEdges.Num(); i += 2)
    {
        SpawnPath(Vertices[TreeEdges[i]], Vertices[TreeEdges[i + 1]]);
    }

    // Move the player spawn point on first platform to not fall 
    AActor* PlayerSpawnPoint = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
    if(PlayerSpawnPoint)
    {
        PlayerSpawnPoint->SetActorLocation(FVector(Vertices[0].X, Vertices[0].Y, PlatformRadius));
    }
}

void AMapGenerator::SpawnPlatform(const FVector2D& Location)
{
    if (PlatformActorClass)
    {
        FVector SpawnLocation(Location.X, Location.Y, 0.0f);
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        APointActor* Platform = GetWorld()->SpawnActor<APointActor>(PlatformActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (Platform)
        {
            Platform->SetActorScale3D(FVector(PlatformRadius / 50.0f));
            Platform->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
}

void AMapGenerator::SpawnPath(const FVector2D& Start, const FVector2D& End)
{
    if (PathActorClass)
    {
        FVector StartLocation(Start.X, Start.Y, 0.0f);
        FVector EndLocation(End.X, End.Y, 0.0f);
        FVector MidPoint = (StartLocation + EndLocation) / 2.0f;
        float PathLength = FVector::Dist(StartLocation, EndLocation);

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        APointActor* Path = GetWorld()->SpawnActor<APointActor>(PathActorClass, MidPoint, FRotator::ZeroRotator, SpawnParams);
        if (Path)
        {
            FRotator PathRotation = UKismetMathLibrary::FindLookAtRotation(StartLocation, EndLocation);
            Path->SetActorRotation(PathRotation);
            Path->SetActorScale3D(FVector(PathLength / 100.0f, PathWidth / 100.0f, 1.0f));
            Path->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
        }
    }
}