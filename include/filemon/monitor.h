#pragma once

#include <string>

#include <filemon/status.h>
#include <filemon/event.h>
#include <filemon/os/file.h>

namespace filemon
{

  // TODO: Let's not use blocking and run the event check on another thread
  class FileMonitor
  {
  private:
#pragma region Private props
    //
    const char *m_fileName;
    FileHandle m_file;

    // We can use this to ensure an attribute change is for the size of a file
    size_t m_fileSize;

// OS-dependent monitoring props
#if FILEMON_TARGET_LINUX
    int m_notifInstance;
    int m_notifWatch;
#endif

#if FILEMON_TARGET_MACOS
    int m_kqueueDesc;
    kevent64_s m_changeList;
    kevent64_s m_eventList;
#endif

//
#pragma endregion

  public:
#pragma region C/dtors
    //

    FileMonitor();

    FileMonitor(std::string &fileName);
    FileMonitor(const char *fileName);
    ~FileMonitor();
//
#pragma endregion

#pragma region Methods
    //

    // Opens the target file
    filemon::Status open();

    // Closes the target file
    filemon::Status close();

    // Wait for file change event. This blocks.
    filemon::Status getEvent(filemon::Event &ev);

//
#pragma endregion
  };

}