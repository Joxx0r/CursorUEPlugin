// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using Microsoft.Win32;

namespace UnrealBuildTool.Rules
{
	public class CursorSourceCodeAccess : ModuleRules
	{
		public CursorSourceCodeAccess(ReadOnlyTargetRules Target) : base(Target)
		{
			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"SourceCodeAccess",
					"DesktopPlatform"
				}
			);

			if (Target.bBuildEditor)
			{
				PrivateDependencyModuleNames.Add("HotReload");
			}
			
			bBuildLocallyWithSNDBS = true;
			ShortName = "CSAC";
		}
	}
}
