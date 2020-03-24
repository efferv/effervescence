//
//  EFF_Utils.cpp
//  effervescence-driver
//
//  Created by Nerrons on 24/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

// Self Include
#include "EFF_Utils.h"

// Local Includes
#include "EFF_Types.h"

// System Includes
#include <MacTypes.h>
#include <mach/mach_error.h>
#include <CoreFoundation/CoreFoundation.h>  // For kCFCoreFoundationVersionNumber


#pragma clang assume_nonnull begin

dispatch_queue_t EFFGetDispatchQueue_PriorityUserInteractive()
{
    long queueClass;

    // Compile-time check that QOS_CLASS_USER_INTERACTIVE can be used. It was added in 10.10.
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101000  // MAC_OS_X_VERSION_10_10
    // Runtime check for the same.
    if(floor(kCFCoreFoundationVersionNumber) > kCFCoreFoundationVersionNumber10_9)
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpartial-availability"
        queueClass = QOS_CLASS_USER_INTERACTIVE;
#pragma clang diagnostic pop
    }
    else
#endif
    {
        // Fallback for older versions.
        queueClass = DISPATCH_QUEUE_PRIORITY_HIGH;
    }

    return dispatch_get_global_queue(queueClass, 0);
}

namespace EFF_Utils
{
    // Forward declarations
    static OSStatus LogAndSwallowExceptions(const char* __nullable fileName,
                                            int lineNumber,
                                            const char* callerName,
                                            const char* __nullable message,
                                            bool expected,
                                            const std::function<void(void)>& function);


#pragma mark Exception utils
    
    bool LogIfMachError(const char* callerName,
                        const char* errorReturnedBy,
                        mach_error_t error)
    {
        if(error != KERN_SUCCESS)
        {
            char* errorStr = mach_error_string(error);
            LogError("%s: %s returned an error (%d): %s\n",
                     callerName,
                     errorReturnedBy,
                     error,
                     errorStr ? errorStr : "Unknown error");
            return false;
        }
        
        return true;
    }

    void ThrowIfMachError(const char* callerName,
                          const char* errorReturnedBy,
                          mach_error_t error)
    {
        if(!LogIfMachError(callerName, errorReturnedBy, error))
        {
            Throw(CAException(error));
        }
    }
    
    OSStatus LogAndSwallowExceptions(const char* __nullable fileName,
                                     int lineNumber,
                                     const char* callerName,
                                     const std::function<void(void)>& function)
    {
        return LogAndSwallowExceptions(fileName, lineNumber, callerName, nullptr, true, function);
    }

    OSStatus LogAndSwallowExceptions(const char* __nullable fileName,
                                     int lineNumber,
                                     const char* callerName,
                                     const char* __nullable message,
                                     const std::function<void(void)>& function)
    {
        return LogAndSwallowExceptions(fileName, lineNumber, callerName, message, true, function);
    }
    
    void LogException(const char* __nullable fileName,
                      int lineNumber,
                      const char* callerName,
                      const CAException& e)
    {
        OSStatus err = e.GetError();
        const char err4CC[5] = CA4CCToCString(err);

        LogError("%s:%d:%s: CAException, code: '%s' (%d).",
                 (fileName ? fileName : ""),
                 lineNumber,
                 callerName,
                 err4CC,
                 err);
    }
    
    void LogUnexpectedException(const char* __nullable fileName,
                                int lineNumber,
                                const char* callerName)
    {
        LogError("%s:%d:%s: Unknown unexpected exception.",
                 (fileName ? fileName : ""),
                 lineNumber,
                 callerName);
    }
    
    OSStatus LogUnexpectedExceptions(const char* callerName,
                                     const std::function<void(void)>& function)
    {
        return LogUnexpectedExceptions(nullptr, -1, callerName, nullptr, function);
    }
    
    OSStatus LogUnexpectedExceptions(const char* __nullable fileName,
                                     int lineNumber,
                                     const char* callerName,
                                     const std::function<void(void)>& function)
    {
        return LogUnexpectedExceptions(fileName, lineNumber, callerName, nullptr, function);
    }
    
    OSStatus LogUnexpectedExceptions(const char* __nullable fileName,
                                     int lineNumber,
                                     const char* callerName,
                                     const char* __nullable message,
                                     const std::function<void(void)>& function)
    {
        return LogAndSwallowExceptions(fileName, lineNumber, callerName, message, false, function);
    }


#pragma mark Implementation

    static OSStatus LogAndSwallowExceptions(const char* __nullable fileName,
                                            int lineNumber,
                                            const char* callerName,
                                            const char* __nullable message,
                                            bool expected,
                                            const std::function<void(void)>& function)
    {
        try
        {
            function();
        }
        catch(const CAException& e)
        {
            // TODO: Can/should we log a stack trace somewhere? (If so, also in the following catch block.)
            // TODO: Log a warning instead of an error for expected exceptions?
            OSStatus err = e.GetError();
            const char err4CC[5] = CA4CCToCString(err);

            LogError("%s:%d:%s: %sCAException, code: '%s' (%d). %s%s %s %s ",
                     (fileName ? fileName : ""),
                     lineNumber,
                     callerName,
                     (expected ? "" : "Unexpected "),
                     err4CC,
                     err,
                     (message ? message : ""),
                     (message ? "." : ""),
                     (expected ? "If you think this might be a bug:"
                               : "Feel free to report this at"),
                     kEFFIssueTrackerURL);
            
#if EFF_StopDebuggerOnLoggedExceptions || EFF_StopDebuggerOnLoggedUnexpectedExceptions
#if !EFF_StopDebuggerOnLoggedExceptions
            if(!expected)
#endif
            {
                EFFAssert(false, "CAException");
            }
#endif
            return e.GetError();
        }
        catch(...)
        {
            LogError("%s:%d:%s: %s exception. %s%s %s %s",
                     (fileName ? fileName : ""),
                     lineNumber,
                     callerName,
                     (expected ? "Unknown" : "Unexpected unknown"),
                     (message ? message : ""),
                     (message ? "." : ""),
                     (expected ? "If you think this might be a bug:"
                               : "Feel free to report this at"),
                     kEFFIssueTrackerURL);

#if EFF_StopDebuggerOnLoggedExceptions || EFF_StopDebuggerOnLoggedUnexpectedExceptions
            EFFAssert(false, "Unknown exception");
#endif
            return -1;
        }
        
        return noErr;
    }
}

#pragma clang assume_nonnull end
