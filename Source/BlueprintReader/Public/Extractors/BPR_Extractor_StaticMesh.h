#pragma once

#include "CoreMinimal.h"

class UStaticMesh;

class BLUEPRINTREADER_API BPR_Extractor_StaticMesh
{
public:
	BPR_Extractor_StaticMesh();
	~BPR_Extractor_StaticMesh();

	void ProcessStaticMesh(UObject* Object, FText& OutText);

private:
	void AppendHeader(UStaticMesh* Mesh, FString& OutText);
	void AppendMaterials(UStaticMesh* Mesh, FString& OutText);
	void AppendLODs(UStaticMesh* Mesh, FString& OutText);
	void AppendSockets(UStaticMesh* Mesh, FString& OutText);
};
