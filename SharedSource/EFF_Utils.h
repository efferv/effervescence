//
//  EFF_Utils.h
//  effervescence-driver
//
//  Created by Nerrons on 24/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_Utils_h
#define EFF_Utils_h

// PublicUtility Includes
#include "CADebugMacros.h"
#if defined(__cplusplus)
#include "CAException.h"

// STL Includes
#include <functional>
#endif /* defined(__cplusplus) */

// System Includes
#include <mach/error.h>
#include <dispatch/dispatch.h>


#pragma mark Macros

// The Assert macro from CADebugMacros with support for format strings and line numbers added.
#if DEBUG
    #define EFFAssert(inCondition, inMessage, ...)                                  \
        if(!(inCondition))                                                          \
        {                                                                           \
            DebugMsg("%s:%d:%s: " inMessage,                                        \
                     __FILE__,                                                      \
                     __LINE__,                                                      \
                     __FUNCTION__,                                                  \
                     ## __VA_ARGS__);                                               \
            __ASSERT_STOP;                                                          \
        }
#else
    #define EFFAssert(inCondition, inMessage, ...)
#endif /* DEBUG */

#define EFFAssertNonNull(expression) \
    EFFAssertNonNull2((expression), #expression)

#define EFFAssertNonNull2(expression, expressionStr) \
    EFFAssert((expression), \
              "%s:%d:%s: '%s' is null", \
              __FILE__, \
              __LINE__, \
              __FUNCTION__, \
              expressionStr);

// Used to give the first 3 arguments of EFF_Utils::LogAndSwallowExceptions and
// EFF_Utils::LogUnexpectedExceptions (and probably others in future). Mainly so we can call those
// functions directly instead of using the macro wrappers.
#define EFFDbgArgs __FILE__, __LINE__, __FUNCTION__


#pragma mark Objective-C Macros

#if defined(__OBJC__)

#if __has_feature(objc_generics)

// This trick is from https://gist.github.com/robb/d55b72d62d32deaee5fa
@interface EFFNonNullCastHelper<__covariant T>

- (nonnull T) asNonNull;

@end

// Explicitly casts expressions from nullable to non-null. Only works with expressions that
// evaluate to Objective-C objects. Use EFF_Utils::NN for other types.
//
// TODO: Replace existing non-null casts with this.
#define EFFNN(expression) ({ \
        __typeof((expression)) value = (expression); \
        EFFAssertNonNull2(value, #expression); \
        EFFNonNullCastHelper<__typeof((expression))>* helper; \
        (__typeof(helper.asNonNull) __nonnull)value; \
    })

#else /* __has_feature(objc_generics) */

#define EFFNN(expression) ({ \
        id value = (expression); \
        EFFAssertNonNull2(value, #expression); \
        value; \
    })

#endif /* __has_feature(objc_generics) */

#endif /* defined(__OBJC__) */


#pragma mark C++ Macros

#if defined(__cplusplus)
#define EFFLogException(exception) \
    EFF_Utils::LogException(__FILE__, __LINE__, __FUNCTION__, exception)

#define EFFLogExceptionIn(callerName, exception) \
    EFF_Utils::LogException(__FILE__, __LINE__, callerName, exception)

#define EFFLogAndSwallowExceptions(callerName, function) \
    EFF_Utils::LogAndSwallowExceptions(__FILE__, __LINE__, callerName, function)

#define EFFLogAndSwallowExceptionsMsg(callerName, message, function) \
    EFF_Utils::LogAndSwallowExceptions(__FILE__, __LINE__, callerName, message, function)

#define EFFLogUnexpectedException() \
    EFF_Utils::LogUnexpectedException(__FILE__, __LINE__, __FUNCTION__)

#define EFFLogUnexpectedExceptionIn(callerName) \
    EFF_Utils::LogUnexpectedException(__FILE__, __LINE__, callerName)

#define EFFLogUnexpectedExceptions(callerName, function) \
    EFF_Utils::LogUnexpectedExceptions(__FILE__, __LINE__, callerName, function)

#define EFFLogUnexpectedExceptionsMsg(callerName, message, function) \
    EFF_Utils::LogUnexpectedExceptions(__FILE__, __LINE__, callerName, message, function)
#endif /* defined(__cplusplus) */


#pragma clang assume_nonnull begin

#pragma mark C Utility Functions

dispatch_queue_t EFFGetDispatchQueue_PriorityUserInteractive(void);


#if defined(__cplusplus)
#pragma mark C++ Utility Functions

namespace EFF_Utils
{
    // Used to explicitly cast from nullable to non-null. For Objective-C objects, use the EFFNN
    // macro (above).
    template <typename T>
    inline T __nonnull NN(T __nullable v) {
        EFFAssertNonNull(v);
        return static_cast<T __nonnull>(v);
    }
    
    // Log (and swallow) errors returned by Mach functions. Returns false if there was an error.
    bool LogIfMachError(const char* callerName,
                        const char* errorReturnedBy,
                        mach_error_t error);
    
    // Similar to ThrowIfKernelError from CADebugMacros.h, but also logs (in debug builds) the
    // Mach error string that corresponds to the error.
    void ThrowIfMachError(const char* callerName,
                          const char* errorReturnedBy,
                          mach_error_t error);
    
    // If function throws an exception, log an error and continue.
    //
    // Fails/stops debug builds. It's likely that if we log an error for an exception in release
    // builds, even if it's expected (i.e. not a bug in Background Music), we'd want to know if
    // it gets thrown during testing/debugging.
    OSStatus LogAndSwallowExceptions(const char* __nullable fileName,
                                     int lineNumber,
                                     const char* callerName,
                                     const std::function<void(void)>& function);
    
    OSStatus LogAndSwallowExceptions(const char* __nullable fileName,
                                     int lineNumber,
                                     const char* callerName,
                                     const char* __nullable message,
                                     const std::function<void(void)>& function);
    
    void     LogException(const char* __nullable fileName,
                          int lineNumber,
                          const char* callerName,
                          const CAException& e);
    
    void     LogUnexpectedException(const char* __nullable fileName,
                                    int lineNumber,
                                    const char* callerName);
    
    OSStatus LogUnexpectedExceptions(const char* callerName,
                                     const std::function<void(void)>& function);
    
    OSStatus LogUnexpectedExceptions(const char* __nullable fileName,
                                     int lineNumber,
                                     const char* callerName,
                                     const std::function<void(void)>& function);
    
    // Log unexpected exceptions and continue.
    //
    // Generally, you don't want to use this unless the alternative is to crash. And even then
    // crashing is often the better option. (Especially if we've added crash reporting by the
    // time you're reading this.)
    //
    // Fails/stops debug builds.
    //
    // TODO: Allow a format string and args for the message.
    OSStatus LogUnexpectedExceptions(const char* __nullable fileName,
                                     int lineNumber,
                                     const char* callerName,
                                     const char* __nullable message,
                                     const std::function<void(void)>& function);
}

#endif /* defined(__cplusplus) */
#pragma clang assume_nonnull end


#endif /* EFF_Utils_h */
