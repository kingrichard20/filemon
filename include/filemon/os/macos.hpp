#pragma once

#include <filemon/os/defs.h>
#include <filemon/monitor.h>

#if FILEMON_TARGET_MACOS
//

#pragma region C/dtor impl

filemon::FileMonitor::FileMonitor(std::string &fileName)
{
}

filemon::FileMonitor::FileMonitor(const char *fileName)
{
}

filemon::FileMonitor::~FileMonitor()
{
}

#pragma endregion

//
#endif