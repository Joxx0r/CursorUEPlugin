#include "CursorSourceCodeAccessModule.h"
#include "CursorSourceCodeAccessor.h"

#define LOCTEXT_NAMESPACE "CursorSourceCodeAccessor"

LLM_DEFINE_TAG(CursorSourceCodeAccessor);

FCursorSourceCodeAccessModule::FCursorSourceCodeAccessModule()
: CursorSourceCodeAccessor(MakeShareable(new FCursorSourceCodeAccessor()))
{
}

void FCursorSourceCodeAccessModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	CursorSourceCodeAccessor->Startup();
	
	IModularFeatures::Get().RegisterModularFeature(TEXT("SourceCodeAccessor"), &CursorSourceCodeAccessor.Get() );
}

void FCursorSourceCodeAccessModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	// unbind provider from editor
	IModularFeatures::Get().UnregisterModularFeature(TEXT("SourceCodeAccessor"), &CursorSourceCodeAccessor.Get());

	CursorSourceCodeAccessor->Shutdown();
}

FCursorSourceCodeAccessor& FCursorSourceCodeAccessModule::GetAccessor()
{
	return CursorSourceCodeAccessor.Get();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCursorSourceCodeAccessModule, CursorCodeSourceAccess)