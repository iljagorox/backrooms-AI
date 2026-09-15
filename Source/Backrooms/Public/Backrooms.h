#pragma once

#include "Modules/ModuleManager.h"

class FBackroomsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};