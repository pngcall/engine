#ifndef ROBLOX_APPLICATION_H
#define ROBLOX_APPLICATION_H

#include <SDL.h>
#include "Core.h"
#include "View.h"

namespace RBX
{
    class FunctionMarshaller;

    class Application
    {
    private:
        SDL_Window* mainWindow = nullptr;
        RBX::FunctionMarshaller* marshaller = nullptr;
        boost::scoped_ptr<RBX::View> mainView;

    public:
        bool initialize(SDL_Window* window);

        void shutdown();

        static bool parseArguments(int argc, char* argv[]);

        static bool loadAppSettings();

        void initializeNewGame();
    };
} // namespace RBX

#endif //ROBLOX_APPLICATION_H