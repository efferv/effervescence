//
//  EFF_Object.hpp
//  effervescence-driver
//
//  Created by Nerrons on 24/3/20.
//  Copyright © 2020 nerrons. All rights reserved.
//

#ifndef EFF_Object_h
#define EFF_Object_h

// System Includes
#include <CoreAudio/AudioServerPlugIn.h>

class EFF_Object
{


#pragma mark Construction/Destruction
    
public:
                        EFF_Object(AudioObjectID inObjectID,
                                   AudioClassID inClassID,
                                   AudioClassID inBaseClassID,
                                   AudioObjectID inOwnerObjectID);
    virtual void        Activate();
    virtual void        Deactivate();

protected:
    virtual             ~EFF_Object();

private:
                        EFF_Object(const EFF_Object&);
    EFF_Object&         operator=(const EFF_Object&);


#pragma mark Attributes
    
public:
    AudioObjectID       GetObjectID()       const { return mObjectID; }
    void*               GetObjectIDAsPtr()  const { uintptr_t thePtr = mObjectID;
                                                    return reinterpret_cast<void*>(thePtr); }
    AudioClassID        GetClassID()        const { return mClassID; }
    AudioClassID        GetBaseClassID()    const { return mBaseClassID; }
    AudioObjectID       GetOwnerObjectID()  const { return mOwnerObjectID; }
    bool                IsActive()          const { return mIsActive; }


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
    virtual void        SetPropertyData(AudioObjectID inObjectID,
                                        pid_t inClientPID,
                                        const AudioObjectPropertyAddress& inAddress,
                                        UInt32 inQualifierDataSize,
                                        const void* inQualifierData,
                                        UInt32 inDataSize,
                                        const void* inData);


#pragma mark Implementation
    
protected:
    AudioObjectID       mObjectID;
    AudioClassID        mClassID;
    AudioClassID        mBaseClassID;
    AudioObjectID       mOwnerObjectID;
    bool                mIsActive;
};

#endif /* EFF_Object_h */
