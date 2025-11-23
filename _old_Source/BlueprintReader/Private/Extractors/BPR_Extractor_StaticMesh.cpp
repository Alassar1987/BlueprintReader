#include "Extractors/BPR_Extractor_StaticMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"

BPR_Extractor_StaticMesh::BPR_Extractor_StaticMesh() {}
BPR_Extractor_StaticMesh::~BPR_Extractor_StaticMesh() {}

void BPR_Extractor_StaticMesh::ProcessStaticMesh(UObject* Object, FText& OutText)
{
#if WITH_EDITOR
    FString Result;

    UStaticMesh* Mesh = Cast<UStaticMesh>(Object);
    if (!Mesh)
    {
        OutText = FText::FromString(TEXT("Object is not a Static Mesh."));
        return;
    }

    Result += TEXT("# Static Mesh\n\n");

    AppendHeader(Mesh, Result);
    AppendMaterials(Mesh, Result);
    AppendLODs(Mesh, Result);
    AppendSockets(Mesh, Result);

    OutText = FText::FromString(Result);
#endif
}

void BPR_Extractor_StaticMesh::AppendHeader(UStaticMesh* Mesh, FString& OutText)
{
    OutText += FString::Printf(TEXT("- Name: %s\n"), *Mesh->GetName());
    OutText += FString::Printf(TEXT("- Path: %s\n"), *Mesh->GetPathName());
    OutText += FString::Printf(TEXT("- LOD Count: %d\n\n"), Mesh->GetNumLODs());
}

void BPR_Extractor_StaticMesh::AppendMaterials(UStaticMesh* Mesh, FString& OutText)
{
    OutText += TEXT("## Materials\n\n");
    for (int i = 0; i < Mesh->GetStaticMaterials().Num(); ++i)
    {
        OutText += FString::Printf(TEXT("- [%d] %s\n"), i, *Mesh->GetMaterial(i)->GetName());
    }
    OutText += TEXT("\n");
}

void BPR_Extractor_StaticMesh::AppendLODs(UStaticMesh* Mesh, FString& OutText)
{
    OutText += TEXT("## LODs\n\n");
    // Тут можно добавить подробности по секциям, треугольникам и UV, если нужно
    for (int i = 0; i < Mesh->GetNumLODs(); ++i)
    {
        OutText += FString::Printf(TEXT("- LOD %d\n"), i);
    }
    OutText += TEXT("\n");
}

void BPR_Extractor_StaticMesh::AppendSockets(UStaticMesh* Mesh, FString& OutText)
{
    if (Mesh->Sockets.Num() == 0) return;

    OutText += TEXT("## Sockets\n\n");
    for (const UStaticMeshSocket* Socket : Mesh->Sockets)
    {
        OutText += FString::Printf(TEXT("- %s (Location: %s)\n"), 
            *Socket->SocketName.ToString(),
            *Socket->RelativeLocation.ToString());
    }
    OutText += TEXT("\n");
}
