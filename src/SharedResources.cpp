#include "SharedResources.hpp"
#include "Common.hpp"
#include "Exec.hpp"
#include "NodeResultPrinting.hpp"
#include "Mutex.hpp"
#include "Atomic.hpp"
#include "BuildQueue.hpp"

namespace t2
{
  static void SharedResourceExecute(const SharedResourceData* sharedResource, const char* action, const char* formatString, MemAllocHeap* heap, int maxNodes)
  {
    const int fullAnnotationLength = strlen(sharedResource->m_Annotation) + 20;
    char* fullAnnotation = (char*)alloca(fullAnnotationLength);
    snprintf(fullAnnotation, fullAnnotationLength, formatString, sharedResource->m_Annotation.Get());

    const int envVarsCount = sharedResource->m_EnvVars.GetCount();
    EnvVariable* envVars = (EnvVariable*)alloca(envVarsCount * sizeof(EnvVariable));
    for (int i = 0; i < envVarsCount; ++i)
    {
      envVars[i].m_Name = sharedResource->m_EnvVars[i].m_Name;
      envVars[i].m_Value = sharedResource->m_EnvVars[i].m_Value;
    }

    uint64_t time_exec_started = TimerGet();
    ExecResult result = ExecuteProcess(action, envVarsCount, envVars, heap, 0, false);
    PrintNonNodeActionResult(TimerDiffSeconds(time_exec_started, TimerGet()), maxNodes, result.m_ReturnCode == 0 ? MessageStatusLevel::Success : MessageStatusLevel::Failure, fullAnnotation, &result);
  }

  static void SharedResourceCreate(const SharedResourceData* sharedResource, MemAllocHeap* heap, int maxNodes)
  {
    if (sharedResource->m_CreateAction != nullptr)
      SharedResourceExecute(sharedResource, sharedResource->m_CreateAction, "Creating %s", heap, maxNodes);
  }

  void SharedResourceAcquire(BuildQueue* queue, MemAllocHeap* heap, uint32_t sharedResourceIndex)
  {
    uint32_t& refVar = queue->m_SharedResourcesCreated[sharedResourceIndex];

    if (refVar == 0)
    {
      MutexLock(&queue->m_SharedResourcesLock);
      // Check that another thread didn't start this resource while we were waiting for the lock
      if (refVar == 0)
      {
        SharedResourceCreate(&queue->m_Config.m_SharedResources[sharedResourceIndex], heap, queue->m_Config.m_MaxNodes);
        AtomicIncrement(&refVar);
      }
      MutexUnlock(&queue->m_SharedResourcesLock);
    }
  }

  void SharedResourceDestroy(BuildQueue* queue, MemAllocHeap* heap, uint32_t sharedResourceIndex)
  {
    const SharedResourceData* sharedResource = &queue->m_Config.m_SharedResources[sharedResourceIndex];
    if (sharedResource->m_DestroyAction != nullptr)
      SharedResourceExecute(sharedResource, sharedResource->m_DestroyAction, "Destroying %s", heap, queue->m_Config.m_MaxNodes);
    queue->m_SharedResourcesCreated[sharedResourceIndex] = 0;
  }
}
