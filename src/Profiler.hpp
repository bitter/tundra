#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <stdint.h>
#include "Exec.hpp"

namespace t2
{
  extern bool g_ProfilerEnabled;

  // Simple non-hierarchical profiler; that dumps output into Google Chrome Tracing
  // JSON format.
  //
  // Begin/End a profiling event with ProfilerBegin and ProfilerEnd, or automatically
  // with ProfilerScope struct. Events can not nest (profiler is not hierarchical).
  // String passed into the event will be copied, and split into "name" and "detail" parts on first
  // space character.

  struct ProfilerEvent
  {
    uint64_t    m_Time;
    uint64_t    m_Duration;
    const char* m_Name;
    const char* m_Info;
    int32_t     m_NodeIndex;
    TundraPID   m_PID;
  };

  void ProfilerInit(const char* fileName, int threadCount);
  void ProfilerDestroy();

  ProfilerEvent* ProfilerBeginImpl(const char* name, int threadIndex);
  void ProfilerEndImpl(int threadIndex);

  inline ProfilerEvent* ProfilerBegin(const char* name, int threadIndex)
  {
    return (g_ProfilerEnabled) ? ProfilerBeginImpl(name, threadIndex) : nullptr;
  }

  inline void ProfilerEnd(int threadIndex)
  {
    if (g_ProfilerEnabled)
      ProfilerEndImpl(threadIndex);
  }

  struct ProfilerScope
  {
    int m_ThreadIndex;
    ProfilerEvent* m_Evt;
    ProfilerScope(const char* name, int threadIndex, int32_t nodeIndex = -1)
    {
      m_ThreadIndex = threadIndex;
      m_Evt = ProfilerBegin(name, threadIndex);
      if (m_Evt != nullptr)
        m_Evt->m_NodeIndex = nodeIndex;
    }

    ~ProfilerScope()
    {
      ProfilerEnd(m_ThreadIndex);
    }
  };

}

#endif
