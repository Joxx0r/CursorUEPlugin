#include "CursorSourceCodeAccessor.h"
#include "CursorSourceCodeAccessModule.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "Misc/UProjectInfo.h"
#include "Misc/App.h"

#if PLATFORM_WINDOWS
#include "Internationalization/Regex.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogCursor, Log, All);

#define LOCTEXT_NAMESPACE "CursorSourceCodeAccessor"

namespace
{
	static const TCHAR* GCursorWorkspaceExtension = TEXT(".code-workspace");
}

static FString MakePath(const FString& InPath)
{
	return TEXT("\"") + InPath + TEXT("\""); 
}

FString FCursorSourceCodeAccessor::GetSolutionPath() const
{
	FScopeLock Lock(&CachedSolutionPathCriticalSection);

	if (IsInGameThread())
	{
		CachedSolutionPath = FPaths::ProjectDir();

		if (!FUProjectDictionary::GetDefault().IsForeignProject(CachedSolutionPath))
		{
			CachedSolutionPath = FPaths::Combine(FPaths::RootDir(), FString("UE5") + GCursorWorkspaceExtension);
		}
		else
		{
			FString BaseName = FApp::HasProjectName() ? FApp::GetProjectName() : FPaths::GetBaseFilename(CachedSolutionPath);
			CachedSolutionPath = FPaths::Combine(CachedSolutionPath, BaseName + GCursorWorkspaceExtension);
		}
	}

	return CachedSolutionPath;
}

/** save all open documents in Cursor, when recompiling */
static void OnModuleCompileStarted(bool bIsAsyncCompile)
{
	FCursorSourceCodeAccessModule& CursorAccessModule = FModuleManager::LoadModuleChecked<FCursorSourceCodeAccessModule>(TEXT("CursorSourceCodeAccess"));
	CursorAccessModule.GetAccessor().SaveAllOpenDocuments();
}

void FCursorSourceCodeAccessor::Startup()
{
	GetSolutionPath();
	RefreshAvailability();
}

void FCursorSourceCodeAccessor::RefreshAvailability()
{
#if PLATFORM_WINDOWS
	FString IDEPath;

	if (!FWindowsPlatformMisc::QueryRegKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Classes\\Applications\\Cursor.exe\\shell\\open\\command\\"), TEXT(""), IDEPath))
	{
		FWindowsPlatformMisc::QueryRegKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Classes\\Applications\\Cursor.exe\\shell\\open\\command\\"), TEXT(""), IDEPath);
	}

	FString PatternString(TEXT("\"(.*)\" \".*\""));
	FRegexPattern Pattern(PatternString);
	FRegexMatcher Matcher(Pattern, IDEPath);
	if (Matcher.FindNext())
	{
		FString URL = Matcher.GetCaptureGroup(1);
		if (FPaths::FileExists(URL))
		{
			Location.URL = URL;
		}
	}
	
#endif
}

void FCursorSourceCodeAccessor::Shutdown()
{
}

bool FCursorSourceCodeAccessor::OpenSourceFiles(const TArray<FString>& AbsoluteSourcePaths)
{
	if (Location.IsValid())
	{
		FString SolutionDir = GetSolutionPath();
		TArray<FString> Args;
		Args.Add(MakePath(SolutionDir));

		for (const FString& SourcePath : AbsoluteSourcePaths)
		{
			Args.Add(MakePath(SourcePath));
		}

		return Launch(Args);
	}

	return false;
}

bool FCursorSourceCodeAccessor::AddSourceFiles(const TArray<FString>& AbsoluteSourcePaths, const TArray<FString>& AvailableModules)
{
	// Cursor doesn't need to do anything when new files are added
	return true;
}

bool FCursorSourceCodeAccessor::OpenFileAtLine(const FString& FullPath, int32 LineNumber, int32 ColumnNumber)
{
	if (Location.IsValid())
	{
		// Column & line numbers are 1-based, so dont allow zero
		LineNumber = LineNumber > 0 ? LineNumber : 1;
		ColumnNumber = ColumnNumber > 0 ? ColumnNumber : 1;

		FString SolutionDir = GetSolutionPath();
		TArray<FString> Args;
		Args.Add(MakePath(SolutionDir));
		Args.Add(TEXT("-g ") + MakePath(FullPath) + FString::Printf(TEXT(":%d:%d"), LineNumber, ColumnNumber));
		return Launch(Args);
	}

	return false;
}

bool FCursorSourceCodeAccessor::CanAccessSourceCode() const
{
	// True if we have any versions of VS installed
	return Location.IsValid();
}

FName FCursorSourceCodeAccessor::GetFName() const
{
	return FName("Cursor");
}

FText FCursorSourceCodeAccessor::GetNameText() const
{
	return LOCTEXT("CursorDisplayName", "Cursor");
}

FText FCursorSourceCodeAccessor::GetDescriptionText() const
{
	return LOCTEXT("CursorDisplayDesc", "Open source code files in Cursor");
}

void FCursorSourceCodeAccessor::Tick(const float DeltaTime)
{
}

bool FCursorSourceCodeAccessor::OpenSolution()
{
	if (Location.IsValid())
	{
		return OpenSolutionAtPath(GetSolutionPath());
	}

	return false;
}

bool FCursorSourceCodeAccessor::OpenSolutionAtPath(const FString& InSolutionPath)
{
	if (Location.IsValid())
	{
		FString SolutionPath = InSolutionPath;

		if (!SolutionPath.EndsWith(GCursorWorkspaceExtension))
		{
			SolutionPath = SolutionPath + GCursorWorkspaceExtension;
		}

		TArray<FString> Args;
		Args.Add(MakePath(SolutionPath));
		return Launch(Args);
	}

	return false;
}

bool FCursorSourceCodeAccessor::DoesSolutionExist() const
{
	return FPaths::FileExists(GetSolutionPath());
}


bool FCursorSourceCodeAccessor::SaveAllOpenDocuments() const
{
	return true;
}

bool FCursorSourceCodeAccessor::Launch(const TArray<FString>& InArgs)
{
	if (Location.IsValid())
	{
		FString ArgsString;
		for (const FString& Arg : InArgs)
		{
			ArgsString.Append(Arg);
			ArgsString.Append(TEXT(" "));
		}

		uint32 ProcessID;
		FProcHandle hProcess = FPlatformProcess::CreateProc(*Location.URL, *ArgsString, true, false, false, &ProcessID, 0, nullptr, nullptr, nullptr);
		return hProcess.IsValid();
	}
	
	return false;
}

#undef LOCTEXT_NAMESPACE
