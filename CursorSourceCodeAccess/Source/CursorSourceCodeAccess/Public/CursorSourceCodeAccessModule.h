#pragma once

#include "Modules/ModuleManager.h"

class FCursorSourceCodeAccessor;

class FCursorSourceCodeAccessModule : public IModuleInterface
{
public:

	FCursorSourceCodeAccessModule();
	
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	FCursorSourceCodeAccessor& GetAccessor();

private:
	TSharedRef<FCursorSourceCodeAccessor> CursorSourceCodeAccessor;

};
