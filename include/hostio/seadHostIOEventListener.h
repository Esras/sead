#pragma once

#include <type_traits>

#include "basis/seadTypes.h"
#include "heap/seadDisposer.h"

namespace sead
{
namespace hostio
{
class NodeEvent;
class PropertyEvent;

// These classes always exist (even in release builds, as can be seen in SMO's RTTI) but
// their functions are only defined in debug or develop builds.

class LifeCheckable
{
#ifdef SEAD_DEBUG
public:
    LifeCheckable()
    {
        mDisposer = new (&mDisposerBuf) DisposeHostIOCaller(this);
        initialize_();
    }

    virtual ~LifeCheckable();

    u32 getCreateID() const { return mCreateID; }
    static LifeCheckable* searchInstanceFromCreateID(u32 createID);

    LifeCheckable(const LifeCheckable&) = delete;
    LifeCheckable& operator=(const LifeCheckable&) = delete;

protected:
    virtual void disposeHostIO();

private:
    class DisposeHostIOCaller : public IDisposer
    {
    public:
        explicit DisposeHostIOCaller(LifeCheckable* instance) : mInstance(instance) {}
        ~DisposeHostIOCaller() override;

    private:
        friend class LifeCheckable;
        LifeCheckable* mInstance;
    };

    void initialize_();
    void disposeHostIOImpl_();

    u32 mCreateID = 0;
    LifeCheckable* mPrev = nullptr;
    LifeCheckable* mNext = nullptr;
    DisposeHostIOCaller* mDisposer = nullptr;
    std::aligned_storage_t<sizeof(DisposeHostIOCaller), alignof(DisposeHostIOCaller)> mDisposerBuf;

    static u32 sCurrentCreateID;
    static LifeCheckable* sTopInstance;
#endif
};

class PropertyEventListener : public LifeCheckable
{
#ifdef SEAD_DEBUG
public:
    virtual void listenPropertyEvent(const PropertyEvent* event) = 0;
#endif
};

class NodeEventListener : public PropertyEventListener
{
#ifdef SEAD_DEBUG
public:
    void listenPropertyEvent(const PropertyEvent* event) override {}
    virtual void listenNodeEvent(const NodeEvent* event) {}
#endif
};
}  // namespace hostio
}  // namespace sead
