using UnrealBuildTool;

public class BlueprintReader : ModuleRules
{
	public BlueprintReader(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public всегда только Core
		PublicDependencyModuleNames.AddRange(
			new string[] 
			{
				"Core"
			}
		);

		// Всё остальное Private
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			}
		);

		// Editor-only зависимости
		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"UnrealEd",
					"ContentBrowser",
					"AssetTools",
					"ToolMenus",
					"BlueprintGraph",
					"GraphEditor",
					"KismetCompiler",
					"WorkspaceMenuStructure",
					"UMG",
					"EditorStyle",
					"StateTreeModule",
					"GameplayStateTreeModule",
					"StateTreeEditorModule"
				}
			);
		}
	}
}