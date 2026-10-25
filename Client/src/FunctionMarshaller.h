#ifndef ROBLOX_FUNCTIONMARSHALLER_H
#define ROBLOX_FUNCTIONMARSHALLER_H

#include "rbx/threadsafe.h"
#include "rbx/CEvent.h"

namespace RBX {
    // A very handy class for marshalling a function across Windows threads (sync and async)
    class FunctionMarshaller {
    public:

    private:
        struct StaticData {
            std::map<DWORD, FunctionMarshaller *> windows;
            boost::recursive_mutex windowsCriticalSection; // TODO: Would non-recursive be safe here?
            ~StaticData();
        };

        SAFE_STATIC(StaticData, staticData)

        rbx::safe_queue<boost::function < void()> *

        >
        asyncCalls;
        int refCount;
        DWORD threadID;

        FunctionMarshaller(DWORD threadID);


        ~FunctionMarshaller();

    public:
        // TODO: Wrap with a reference counter and then remove ~StaticData() cleanup code and remove ReleaseWindow()
        static FunctionMarshaller *GetWindow();

        static void ReleaseWindow(FunctionMarshaller *window);

        static void handleAppEvent(void *pClosure);

        static void freeAppEvent(void *pClosure);

        struct Closure {
            boost::function<void()> *f;
            std::string errorMessage;
            RBX::CEvent *waitEvent;
        };

        void Execute(boost::function<void()> job, CEvent *waitEvent);

        void Submit(boost::function<void()> job);

        // Call this only from the Window's thread
        void ProcessMessages();
    };
}

#endif //ROBLOX_FUNCTIONMARSHALLER_H