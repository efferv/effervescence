//
//  EFF_TestUtils.h
//  effervescence-driver
//
//  Created by Nerrons on 24/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_TestUtils_h
#define EFF_TestUtils_h

// Test Framework
#import <XCTest/XCTest.h>

#if defined(__cplusplus)

// STL Includes
#include <functional>


// Fails the test if f doesn't throw ExpectedException when run.
// (The "self" argument is required by XCTFail, presumably so it can report the context.)
template<typename ExpectedException>
void EFFShouldThrow(XCTestCase* self, const std::function<void()>& f)
{
    try
    {
        f();
        XCTFail();
    }
    catch (ExpectedException)
    { }
}

#endif /* defined(__cplusplus) */


#endif /* EFF_TestUtils_h */
