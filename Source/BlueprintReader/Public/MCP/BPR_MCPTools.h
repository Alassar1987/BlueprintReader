// Copyright (c) 2026 Racoon Coder. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Registers BlueprintReader MCP tools with Unreal MCP (UE 5.8+ only; no-op otherwise).
 * Implemented via IModelContextProtocolTool (direct registration) rather than UToolsetDefinition
 * so the MCP code can be version-gated behind BPR_HAS_MCP without UHT preprocessor constraints.
 *
 * Call RegisterBlueprintReaderMCPTools() once at module startup and
 * UnregisterBlueprintReaderMCPTools() at shutdown.
 */
void RegisterBlueprintReaderMCPTools();
void UnregisterBlueprintReaderMCPTools();
