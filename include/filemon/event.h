#pragma once

namespace filemon
{

  enum EventType
  {
    Write,
    Truncation,
    Deletion,
    Move
  };

  struct Event
  {
    EventType type;
  };

}
