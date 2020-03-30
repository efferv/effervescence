//
//  EFF_ClientTasks.h
//  effervescence-core
//
//  Created by Nerrons on 26/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//
//  The interface between the client classes (EFF_Client, EFF_Clients and EFF_ClientMap) and EFF_TaskQueue.
//

#ifndef EFF_ClientTasks_h
#define EFF_ClientTasks_h

// Local Includes
#include "EFF_Clients.h"
#include "EFF_ClientMap.h"


// Forward Declarations
class EFF_TaskQueue;


#pragma clang assume_nonnull begin

class EFF_ClientTasks
{
    friend class EFF_TaskQueue;

private:
    static bool                 StartIONonRT(EFF_Clients* inClients, UInt32 inClientID)
                                    { return inClients->StartIONonRT(inClientID); }
    static bool                 StopIONonRT(EFF_Clients* inClients, UInt32 inClientID)
                                    { return inClients->StopIONonRT(inClientID); }
    static void                 SwapInShadowMapsRT(EFF_ClientMap* inClientMap)
                                    { inClientMap->SwapInShadowMapsRT(); }
};

#pragma clang assume_nonnull end


#endif /* EFF_ClientTasks_h */
