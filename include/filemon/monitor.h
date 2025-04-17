#pragma once

#include <string>

#include <filemon/status.h>
#include <filemon/event.h>
#include <filemon/os/file.h>

namespace filemon
{

  class FileMonitor
  {
  private:
#pragma region Private props
    //
    const char *m_fileName;
    FileHandle m_file;
//
#pragma endregion

  public:
#pragma region C/dtors
    //
    FileMonitor(std::string &fileName);
    FileMonitor(const char *fileName);
    ~FileMonitor();
//
#pragma endregion

#pragma region Methods
    //

    // Opens the file
    filemon::Status open();

    // Closes the file
    filemon::Status close();

    // Wait for file change event. This blocks
    filemon::Status getEvent(filemon::Event &ev);

//
#pragma endregion
  };

}