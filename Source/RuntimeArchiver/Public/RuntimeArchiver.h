// Georgy Treshchev 2022.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FRuntimeArchiverModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
