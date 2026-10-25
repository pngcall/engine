#include "FunctionMarshaller.h"

#undef min
#undef max

#include "Util/standardout.h"

#include "rbx/Boost.hpp"
#include <SDL.h>

#include <utility>

// ----------------------------------------------------------------------------

using namespace RBX;

FunctionMarshaller::FunctionMarshaller(DWORD threadID)
    : refCount(0)
{
    this->threadID = threadID;
}

FunctionMarshaller::~FunctionMarshaller()
{
    boost::function<void()>* f;
    while (asyncCalls.pop_if_present(f))
        delete f;

    RBXASSERT(threadID == GetCurrentThreadId());

#ifdef _DEBUG
    {
        boost::recursive_mutex::scoped_lock lock(staticData().windowsCriticalSection);
        RBXASSERT(refCount == 0);
        // Nobody is using this window
        RBXASSERT(staticData().windows.find(threadID) == staticData().windows.end());
    }
#endif
}


FunctionMarshaller* FunctionMarshaller::GetWindow()
{
    // Share a common FunctionMarshaller in a given Thread

    boost::recursive_mutex::scoped_lock lock(staticData().windowsCriticalSection);
    DWORD threadID = GetCurrentThreadId();
    auto find = staticData().windows.find(threadID);
    if (find != staticData().windows.end())
    {
        // We already created a window, so use it again
        find->second->refCount++;
        return find->second;
    }
    else
    {
        // Create a new window
        auto* window = new FunctionMarshaller(threadID);
        staticData().windows[threadID] = window;
        window->refCount++;
        return window;
    }
}

void FunctionMarshaller::ReleaseWindow(FunctionMarshaller* window)
{
    boost::recursive_mutex::scoped_lock lock(staticData().windowsCriticalSection);
    window->refCount--;
    if (window->refCount == 0)
    {
        // Nobody is using this window
        staticData().windows.erase(window->threadID);
        //		window->DestroyWindow();
    }
}

void FunctionMarshaller::handleAppEvent(void* pClosure)
{
    auto* closure = (FunctionMarshaller::Closure*)pClosure;
    RBX::CEvent* pWaitEvent = closure->waitEvent;
    try
    {
        boost::function<void()>* pF = closure->f;
        (*pF)();

        delete pF;
        delete closure;
    }
    catch (RBX::base_exception& e)
    {
        StandardOut::singleton()->print(RBX::MESSAGE_ERROR, e);
        closure->errorMessage = e.what();
    }


    // If a task is waiting on an event, set it
    if (pWaitEvent)
    {
        pWaitEvent->Set();
    }
}

void FunctionMarshaller::freeAppEvent(void* pClosure)
{
    auto* closure = (FunctionMarshaller::Closure*)pClosure;
    boost::function<void()>* pF = closure->f;
    delete pF;
    delete closure;
}

void FunctionMarshaller::Execute(boost::function<void()> job, CEvent* waitEvent)
{
    if (threadID == GetCurrentThreadId())
    {
        job();
    }
    else
    {
        auto* pClosure = new Closure;
        pClosure->f = new boost::function<void()>(job);
        pClosure->waitEvent = waitEvent;

        SDL_Event event;
        event.type = SDL_USEREVENT;
        event.user.code = 0xc10554e;
        event.user.data1 = pClosure;
        SDL_PushEvent(&event);
    }
}

void FunctionMarshaller::Submit(boost::function<void()> job)
{
    auto* pClosure = new Closure;
    pClosure->f = new boost::function<void()>(std::move(job));
    pClosure->waitEvent = nullptr;

    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = 0xc10554e;
    event.user.data1 = pClosure;
    SDL_PushEvent(&event);
}

void FunctionMarshaller::ProcessMessages()
{
}

FunctionMarshaller::StaticData::~StaticData()
{
}