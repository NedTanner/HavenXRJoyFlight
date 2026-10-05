

using UnrealBuildTool;
using System.Collections.Generic;

public class HavenXRJoyFlightEditorTarget : TargetRules
{
	public HavenXRJoyFlightEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "HavenXRJoyFlight" } );
	}
}
