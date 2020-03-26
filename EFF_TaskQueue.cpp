//
//  EFF_TaskQueue.cpp
//  effervescence-driver
//
//  Created by Nerrons on 25/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

// Self Include
#include "EFF_TaskQueue.h"

// Local Includes
#include "EFF_Types.h"
#include "EFF_Utils.h"
#include "EFF_PlugIn.h"
#include "EFF_Clients.h"
#include "EFF_ClientMap.h"
#include "EFF_ClientTasks.h"

// PublicUtility Includes
#include "CAException.h"
#include "CADebugMacros.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#include "CAAtomic.h"
#pragma clang diagnostic pop

// System Includes
#include <mach/mach_init.h>
#include <mach/mach_time.h>
#include <mach/task.h>


#pragma mark Construction/destruction

EFF_TaskQueue::EFF_TaskQueue()
:
    // The inline documentation for thread_time_constraint_policy.period says "A value of 0 indicates that there is no
    // inherent periodicity in the computation". So I figure setting the period to 0 means the scheduler will take as long
    // as it wants to wake our real-time thread, which is fine for us, but once it has only other real-time threads can
    // preempt us. (And that's only if they won't make our computation take longer than kRealTimeThreadMaximumComputationNs).
    mRealTimeThread(/* inThreadRoutine = */ &EFF_TaskQueue::RealTimeThreadProc,
                    /* inParameter */       this,
                    /* inPeriod = */        0,
                    /* inComputation */     NanosToAbsoluteTime(kRealTimeThreadNominalComputationNs),
                    /* inConstraint */      NanosToAbsoluteTime(kRealTimeThreadMaximumComputationNs),
                    /* inIsPreemptible = */ true),
    mNonRealTimeThread(&EFF_TaskQueue::NonRealTimeThreadProc, this)
{
    // Init the semaphores
    auto createSemaphore = [] () {
        semaphore_t theSemaphore;
        kern_return_t theError = semaphore_create(mach_task_self(),
                                                  &theSemaphore,
                                                  SYNC_POLICY_FIFO, 0);
        EFF_Utils::ThrowIfMachError("EFF_TaskQueue::EFF_TaskQueue",
                                    "semaphore_create",
                                    theError);
        ThrowIf(theSemaphore == SEMAPHORE_NULL,
                CAException(kAudioHardwareUnspecifiedError),
                "EFF_TaskQueue::EFF_TaskQueue: Could not create semaphore");
        
        return theSemaphore;
    };
    mRealTimeThreadWorkQueuedSemaphore              = createSemaphore();
    mNonRealTimeThreadWorkQueuedSemaphore           = createSemaphore();
    mRealTimeThreadSyncTaskCompletedSemaphore       = createSemaphore();
    mNonRealTimeThreadSyncTaskCompletedSemaphore    = createSemaphore();
    
    // Pre-allocate enough tasks in mNonRealTimeThreadTasksFreeList that the real-time threads should never have to
    // allocate memory when adding a task to the non-realtime queue.
    for(UInt32 i = 0; i < kNonRealTimeThreadTaskBufferSize; i++)
    {
        EFF_Task* theTask = new EFF_Task;
        mNonRealTimeThreadTasksFreeList.push_NA(theTask);
    }
    
    // Start the worker threads
    mRealTimeThread.Start();
    mNonRealTimeThread.Start();
}

EFF_TaskQueue::~EFF_TaskQueue()
{
    // Join the worker threads
    EFFLogAndSwallowExceptionsMsg("EFF_TaskQueue::~EFF_TaskQueue", "QueueSync", ([&] {
        QueueSync(kEFFTaskStopWorkerThread, /* inRunOnRealtimeThread = */ true);
        QueueSync(kEFFTaskStopWorkerThread, /* inRunOnRealtimeThread = */ false);
    }));

    // Destroy the semaphores
    auto destroySemaphore = [] (semaphore_t inSemaphore) {
        kern_return_t theError = semaphore_destroy(mach_task_self(), inSemaphore);
        EFF_Utils::LogIfMachError("EFF_TaskQueue::~EFF_TaskQueue",
                                  "semaphore_destroy",
                                  theError);
    };
    
    destroySemaphore(mRealTimeThreadWorkQueuedSemaphore);
    destroySemaphore(mNonRealTimeThreadWorkQueuedSemaphore);
    destroySemaphore(mRealTimeThreadSyncTaskCompletedSemaphore);
    destroySemaphore(mNonRealTimeThreadSyncTaskCompletedSemaphore);
    
    EFF_Task* theTask;
    
    // Delete the tasks in the non-realtime tasks free list
    while((theTask = mNonRealTimeThreadTasksFreeList.pop_atomic()) != NULL)
    {
        delete theTask;
    }
    
    // Delete any tasks left on the non-realtime queue that need to be
    while((theTask = mNonRealTimeThreadTasks.pop_atomic()) != NULL)
    {
        if(!theTask->IsSync())
        {
            delete theTask;
        }
    }
}

//static
UInt32  EFF_TaskQueue::NanosToAbsoluteTime(UInt32 inNanos)
{
    // Converts a duration from nanoseconds to absolute time (i.e. number of bus cycles). Used for calculating
    // the real-time thread's time constraint policy.
    
    mach_timebase_info_data_t theTimebaseInfo;
    mach_timebase_info(&theTimebaseInfo);
    
    Float64 theTicksPerNs = static_cast<Float64>(theTimebaseInfo.denom) / theTimebaseInfo.numer;
    return static_cast<UInt32>(inNanos * theTicksPerNs);
}


#pragma mark Task queueing

void    EFF_TaskQueue::QueueSync_SwapClientShadowMaps(EFF_ClientMap* inClientMap)
{
    // TODO: Is there any reason to use uintptr_t when we pass pointers to tasks like this? I can't think of any
    //       reason for a system to have (non-function) pointers larger than 64-bit, so I figure they should fit.
    //
    //       From http://en.cppreference.com/w/cpp/language/reinterpret_cast:
    //       "A pointer converted to an integer of sufficient size and back to the same pointer type is guaranteed
    //        to have its original value [...]"
    QueueSync(kEFFTaskSwapClientShadowMaps,
              /* inRunOnRealtimeThread = */ true,
              reinterpret_cast<UInt64>(inClientMap));
}

void    EFF_TaskQueue::QueueAsync_SendPropertyNotification(AudioObjectPropertySelector inProperty,
                                                           AudioObjectID inDeviceID)
{
    DebugMsg("EFF_TaskQueue::QueueAsync_SendPropertyNotification: Queueing property notification. inProperty=%u inDeviceID=%u",
             inProperty,
             inDeviceID);
    EFF_Task theTask(kEFFTaskSendPropertyNotification,
                     /* inIsSync = */ false,
                     inProperty,
                     inDeviceID);
    QueueOnNonRealtimeThread(theTask);
}

bool    EFF_TaskQueue::Queue_UpdateClientIOState(bool inSync,
                                                 EFF_Clients* inClients,
                                                 UInt32 inClientID,
                                                 bool inDoingIO)
{
    DebugMsg("EFF_TaskQueue::Queue_UpdateClientIOState: Queueing %s %s",
             (inDoingIO ? "kEFFTaskStartClientIO" : "kEFFTaskStopClientIO"),
             (inSync ? "synchronously" : "asynchronously"));
    
    EFF_TaskID theTaskID = (inDoingIO ? kEFFTaskStartClientIO : kEFFTaskStopClientIO);
    UInt64 theClientsPtrArg = reinterpret_cast<UInt64>(inClients);
    UInt64 theClientIDTaskArg = static_cast<UInt64>(inClientID);
    
    if(inSync)
    {
        return QueueSync(theTaskID,
                         false,
                         theClientsPtrArg,
                         theClientIDTaskArg);
    }
    else
    {
        EFF_Task theTask(theTaskID,
                         /* inIsSync = */ false,
                         theClientsPtrArg,
                         theClientIDTaskArg);
        QueueOnNonRealtimeThread(theTask);
        
        // This method's return value isn't used when queueing async, because we can't know what it should be yet.
        return false;
    }
}

// This function happens synchronously, but it can add tasks to either RT or nonRT queues on respective threads
UInt64    EFF_TaskQueue::QueueSync(EFF_TaskID inTaskID,
                                   bool inRunOnRealtimeThread,
                                   UInt64 inTaskArg1,
                                   UInt64 inTaskArg2)
{
    DebugMsg("EFF_TaskQueue::QueueSync: Queueing task synchronously to be processed on the %s thread. inTaskID=%d inTaskArg1=%llu inTaskArg2=%llu",
             (inRunOnRealtimeThread ? "realtime" : "non-realtime"),
             inTaskID,
             inTaskArg1,
             inTaskArg2);
    
    // Create the task
    EFF_Task theTask(inTaskID,
                     /* inIsSync = */ true,
                     inTaskArg1,
                     inTaskArg2);

    // Add the task to the queue
    TAtomicStack<EFF_Task>& theTasks = (inRunOnRealtimeThread
                                        ? mRealTimeThreadTasks
                                        : mNonRealTimeThreadTasks);
    theTasks.push_atomic(&theTask);

    // Wake the worker thread so it'll process the task. (Note that semaphore_signal has an implicit barrier.)
    kern_return_t theError = semaphore_signal(inRunOnRealtimeThread
                                              ? mRealTimeThreadWorkQueuedSemaphore
                                              : mNonRealTimeThreadWorkQueuedSemaphore);
    EFF_Utils::ThrowIfMachError("EFF_TaskQueue::QueueSync",
                                "semaphore_signal",
                                theError);
    
    // Wait until the task has been processed.
    //
    // The worker thread signals all threads waiting on this semaphore when it finishes a task.
    // The comments in WorkerThreadProc explain why we have to check the condition in a loop here.
    bool didLogTimeoutMessage = false;
    while(!theTask.IsComplete())
    {
        semaphore_t theTaskCompletedSemaphore = inRunOnRealtimeThread
                                                ? mRealTimeThreadSyncTaskCompletedSemaphore
                                                : mNonRealTimeThreadSyncTaskCompletedSemaphore;
        // TODO: Because the worker threads use semaphore_signal_all instead of semaphore_signal,
        // a thread can miss the signal if it isn't waiting at the right time.
        // Using a timeout for now as a temporary fix so threads don't get stuck here.
        theError = semaphore_timedwait(theTaskCompletedSemaphore,
                                       (mach_timespec_t){ 0, kRealTimeThreadMaximumComputationNs * 4 });
        
        if(theError == KERN_OPERATION_TIMED_OUT)
        {
            if(!didLogTimeoutMessage && inRunOnRealtimeThread)
            {
                DebugMsg("EFF_TaskQueue::QueueSync: Task %d taking longer than expected.", theTask.GetTaskID());
                didLogTimeoutMessage = true;
            }
        }
        else
        {
            EFF_Utils::ThrowIfMachError("EFF_TaskQueue::QueueSync", "semaphore_timedwait", theError);
        }
        
        CAMemoryBarrier();
    }
    
    if(didLogTimeoutMessage)
    {
        DebugMsg("EFF_TaskQueue::QueueSync: Late task %d finished.", theTask.GetTaskID());
    }
    
    if(theTask.GetReturnValue() != INT64_MAX)
    {
        DebugMsg("EFF_TaskQueue::QueueSync: Task %d returned %llu.", theTask.GetTaskID(), theTask.GetReturnValue());
    }
    
    return theTask.GetReturnValue();
}

void   EFF_TaskQueue::QueueOnNonRealtimeThread(EFF_Task inTask)
{
    // Add the task to our task list
    EFF_Task* freeTask = mNonRealTimeThreadTasksFreeList.pop_atomic();
    
    if(freeTask == NULL)
    {
        LogWarning("EFF_TaskQueue::QueueOnNonRealtimeThread: No pre-allocated tasks left in the free list. Allocating new task.");
        freeTask = new EFF_Task;
    }
    
    *freeTask = inTask;
    
    mNonRealTimeThreadTasks.push_atomic(freeTask);
    
    // Signal the worker thread to process the task. (Note that semaphore_signal has an implicit barrier.)
    kern_return_t theError = semaphore_signal(mNonRealTimeThreadWorkQueuedSemaphore);
    EFF_Utils::ThrowIfMachError("EFF_TaskQueue::QueueOnNonRealtimeThread", "semaphore_signal", theError);
}


#pragma mark Worker threads

void    EFF_TaskQueue::AssertCurrentThreadIsRTWorkerThread(const char* inCallerMethodName)
{
#if DEBUG  // This Assert macro always checks the condition, even in release builds if the compiler doesn't optimise it away
    if(!mRealTimeThread.IsCurrentThread())
    {
        DebugMsg("%s should only be called on the realtime worker thread.", inCallerMethodName);
        __ASSERT_STOP;  // TODO: Figure out a better way to assert with a formatted message
    }
    
    Assert(mRealTimeThread.IsTimeConstraintThread(), "mRealTimeThread should be in a time-constraint priority band.");
#else
    #pragma unused (inCallerMethodName)
#endif
}

//static
void* __nullable    EFF_TaskQueue::RealTimeThreadProc(void* inRefCon)
{
    DebugMsg("EFF_TaskQueue::RealTimeThreadProc: The realtime worker thread has started");
    
    EFF_TaskQueue* refCon = static_cast<EFF_TaskQueue*>(inRefCon);
    refCon->WorkerThreadProc(refCon->mRealTimeThreadWorkQueuedSemaphore,
                             refCon->mRealTimeThreadSyncTaskCompletedSemaphore,
                             &refCon->mRealTimeThreadTasks,
                             NULL,
                             [&] (EFF_Task* inTask) { return refCon->ProcessRealTimeThreadTask(inTask); });
    
    return NULL;
}

//static
void* __nullable    EFF_TaskQueue::NonRealTimeThreadProc(void* inRefCon)
{
    DebugMsg("EFF_TaskQueue::NonRealTimeThreadProc: The non-realtime worker thread has started");
    
    EFF_TaskQueue* refCon = static_cast<EFF_TaskQueue*>(inRefCon);
    refCon->WorkerThreadProc(refCon->mNonRealTimeThreadWorkQueuedSemaphore,
                             refCon->mNonRealTimeThreadSyncTaskCompletedSemaphore,
                             &refCon->mNonRealTimeThreadTasks,
                             &refCon->mNonRealTimeThreadTasksFreeList,
                             [&] (EFF_Task* inTask) { return refCon->ProcessNonRealTimeThreadTask(inTask); });
    
    return NULL;
}

void    EFF_TaskQueue::WorkerThreadProc(semaphore_t inWorkQueuedSemaphore,
                                        semaphore_t inSyncTaskCompletedSemaphore,
                                        TAtomicStack<EFF_Task>* inTasks,
                                        TAtomicStack2<EFF_Task>* __nullable inFreeList,
                                        std::function<bool(EFF_Task*)> inProcessTask)
{
    bool theThreadShouldStop = false;
    
    while(!theThreadShouldStop)
    {
        // Wait until a thread signals that it's added tasks to the queue.
        //
        // Note that we don't have to hold any lock before waiting. If the semaphore is signalled before we begin waiting
        // we'll still get the signal after we do.
        kern_return_t theError = semaphore_wait(inWorkQueuedSemaphore);
        EFF_Utils::ThrowIfMachError("EFF_TaskQueue::WorkerThreadProc", "semaphore_wait", theError);
        
        // Fetch the tasks from the queue.
        //
        // The tasks need to be processed in the order they were added to the queue. Since pop_all_reversed is atomic,
        // other threads can't add new tasks while we're reading, which would mix up the order.
        EFF_Task* theTask = inTasks->pop_all_reversed();
        
        while(theTask != NULL &&
              !theThreadShouldStop)  // Stop processing tasks if we're shutting down
        {
            EFF_Task* theNextTask = theTask->mNext;
            
            EFFAssert(!theTask->IsComplete(),
                      "EFF_TaskQueue::WorkerThreadProc: Cannot process already completed task (ID %d)",
                      theTask->GetTaskID());
            
            EFFAssert(theTask != theNextTask,
                      "EFF_TaskQueue::WorkerThreadProc: EFF_Task %p (ID %d) was added to %s multiple times. arg1=%llu arg2=%llu",
                      theTask,
                      theTask->GetTaskID(),
                      (inTasks == &mRealTimeThreadTasks ? "mRealTimeThreadTasks" : "mNonRealTimeThreadTasks"),
                      theTask->GetArg1(),
                      theTask->GetArg2());
            
            // Process the task
            theThreadShouldStop = inProcessTask(theTask);
            
            // If the task was queued synchronously, let the thread that queued it know we're finished
            if(theTask->IsSync())
            {
                // Marking the task as completed allows QueueSync to return,
                // which means it's possible for theTask to point to invalid memory after this point.
                CAMemoryBarrier();
                theTask->MarkCompleted();
                
                // Signal any threads waiting for their task to be processed.
                //
                // We use semaphore_signal_all instead of semaphore_signal to avoid a race condition in QueueSync.
                // It's possible for threads calling QueueSync to wait on the semaphore in an order
                // different to the order of the tasks they just added to the queue.
                // So after each task is completed we have every waiting thread check if it was theirs.
                //
                // Note that semaphore_signal_all has an implicit barrier.
                theError = semaphore_signal_all(inSyncTaskCompletedSemaphore);
                EFF_Utils::ThrowIfMachError("EFF_TaskQueue::WorkerThreadProc", "semaphore_signal_all", theError);
            }
            else if(inFreeList != NULL)
            {
                // After completing an async task, move it to the free list so the memory can be reused
                inFreeList->push_atomic(theTask);
            }
            
            theTask = theNextTask;
        }
    }
}

// Returns true if the task is to stop the thread, false otherwise
bool    EFF_TaskQueue::ProcessRealTimeThreadTask(EFF_Task* inTask)
{
    AssertCurrentThreadIsRTWorkerThread("EFF_TaskQueue::ProcessRealTimeThreadTask");

    switch(inTask->GetTaskID())
    {
        case kEFFTaskStopWorkerThread:
            DebugMsg("EFF_TaskQueue::ProcessRealTimeThreadTask: Stopping");
            return true;
            
        case kEFFTaskSwapClientShadowMaps:
            {
                DebugMsg("EFF_TaskQueue::ProcessRealTimeThreadTask: Swapping the shadow maps in EFF_ClientMap");
                EFF_ClientMap* theClientMap = reinterpret_cast<EFF_ClientMap*>(inTask->GetArg1());
                EFF_ClientTasks::SwapInShadowMapsRT(theClientMap);
            }
            break;
            
        default:
            Assert(false, "EFF_TaskQueue::ProcessRealTimeThreadTask: Unexpected task ID");
            break;
    }
    
    return false;
}

// Returns true if the task is to stop the thread, false otherwise
bool    EFF_TaskQueue::ProcessNonRealTimeThreadTask(EFF_Task* inTask)
{
#if DEBUG  // Always checks the condition, if for some reason the compiler doesn't optimise it away, even in release builds
    Assert(mNonRealTimeThread.IsCurrentThread(), "ProcessNonRealTimeThreadTask should only be called on the non-realtime worker thread.");
    Assert(mNonRealTimeThread.IsTimeShareThread(), "mNonRealTimeThread should not be in a time-constraint priority band.");
#endif
    
    switch(inTask->GetTaskID())
    {
        case kEFFTaskStopWorkerThread:
            DebugMsg("EFF_TaskQueue::ProcessNonRealTimeThreadTask: Stopping");
            return true;
            
        case kEFFTaskStartClientIO:
            DebugMsg("EFF_TaskQueue::ProcessNonRealTimeThreadTask: Processing kEFFTaskStartClientIO");
            try
            {
                EFF_Clients* theClients = reinterpret_cast<EFF_Clients*>(inTask->GetArg1());
                bool didStartIO = EFF_ClientTasks::StartIONonRT(theClients, static_cast<UInt32>(inTask->GetArg2()));
                inTask->SetReturnValue(didStartIO);
            }
            // TODO: Catch the other types of exceptions EFF_ClientTasks::StartIONonRT can throw here as well.
            // Set the task's return value (rather than rethrowing) so the exceptions can be handled
            // if the task was queued sync.
            // Then QueueSync_StartClientIO can throw some exception and EFF_StartIO can return
            // an appropriate error code to the HAL, instead of the driver just crashing.
            // Do the same for the kEFFTaskStopClientIO case below. And should we set a return value in the catch block for
            // EFF_InvalidClientException as well, so it can also be rethrown in QueueSync_StartClientIO and then handled?
            catch(EFF_InvalidClientException)
            {
                DebugMsg("EFF_TaskQueue::ProcessNonRealTimeThreadTask: Ignoring EFF_InvalidClientException thrown by StartIONonRT. %s",
                         "It's possible the client was removed before this task was processed.");
            }
            break;

        case kEFFTaskStopClientIO:
            DebugMsg("EFF_TaskQueue::ProcessNonRealTimeThreadTask: Processing kEFFTaskStopClientIO");
            try
            {
                EFF_Clients* theClients = reinterpret_cast<EFF_Clients*>(inTask->GetArg1());
                bool didStopIO = EFF_ClientTasks::StopIONonRT(theClients, static_cast<UInt32>(inTask->GetArg2()));
                inTask->SetReturnValue(didStopIO);
            }
            catch(EFF_InvalidClientException)
            {
                DebugMsg("EFF_TaskQueue::ProcessNonRealTimeThreadTask: Ignoring EFF_InvalidClientException thrown by StopIONonRT. %s",
                         "It's possible the client was removed before this task was processed.");
            }
            break;

        case kEFFTaskSendPropertyNotification:
            DebugMsg("EFF_TaskQueue::ProcessNonRealTimeThreadTask: Processing kEFFTaskSendPropertyNotification");
            {
                AudioObjectPropertyAddress thePropertyAddress[] = {
                    { static_cast<UInt32>(inTask->GetArg1()),
                        kAudioObjectPropertyScopeGlobal,
                        kAudioObjectPropertyElementMaster } };
                EFF_PlugIn::Host_PropertiesChanged(static_cast<AudioObjectID>(inTask->GetArg2()), 1, thePropertyAddress);
            }
            break;
            
        default:
            Assert(false, "EFF_TaskQueue::ProcessNonRealTimeThreadTask: Unexpected task ID");
            break;
    }
    
    return false;
}

#pragma clang assume_nonnull end
