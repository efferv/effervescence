//
//  EFF_Control.h
//  effervescence-core
//
//  Created by Nerrons on 25/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_Control_h
#define EFF_Control_h

// Superclass Includes
#include "EFF_Object.h"


#pragma clang assume_nonnull begin

class EFF_Control
:
    public EFF_Object
{

    
#pragma mark Construction/Destruction
    
protected:
                        EFF_Control(AudioObjectID inObjectID,
                                    AudioClassID inClassID,
                                    AudioClassID inBaseClassID,
                                    AudioObjectID inOwnerObjectID,
                                    AudioObjectPropertyScope inScope =
                                            kAudioObjectPropertyScopeOutput,
                                    AudioObjectPropertyElement inElement =
                                            kAudioObjectPropertyElementMaster);
    
    
#pragma mark Property Operations

public:
    virtual bool        HasProperty(AudioObjectID inObjectID,
                                    pid_t inClientPID,
                                    const AudioObjectPropertyAddress& inAddress) const;
    virtual bool        IsPropertySettable(AudioObjectID inObjectID,
                                           pid_t inClientPID,
                                           const AudioObjectPropertyAddress& inAddress) const;
    virtual UInt32      GetPropertyDataSize(AudioObjectID inObjectID,
                                            pid_t inClientPID,
                                            const AudioObjectPropertyAddress& inAddress,
                                            UInt32 inQualifierDataSize,
                                            const void* inQualifierData) const;
    virtual void        GetPropertyData(AudioObjectID inObjectID,
                                        pid_t inClientPID,
                                        const AudioObjectPropertyAddress& inAddress,
                                        UInt32 inQualifierDataSize,
                                        const void* inQualifierData,
                                        UInt32 inDataSize,
                                        UInt32& outDataSize,
                                        void* outData) const;


#pragma mark Implementation

protected:
    void                                CheckObjectID(AudioObjectID inObjectID) const;
    const AudioObjectPropertyScope      mScope;
    const AudioObjectPropertyElement    mElement;

};

#pragma clang assume_nonnull end

#endif /* EFF_Control_h */
