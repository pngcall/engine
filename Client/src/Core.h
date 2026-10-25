#ifndef ROBLOX_CORE_H
#define ROBLOX_CORE_H

#include "Util/standardout.h"
#include "v8datamodel/TeleportService.h"
#include "v8datamodel/FastLogSettings.h"

#include <SDL.h>

namespace RBX {
    class Game;
}

class Core {
private:
    static shared_ptr<RBX::TeleportCallback> callback;
    static SDL_Window *theInstance;
    static bool initialized;

    static void onMessageOut(const RBX::StandardOutMessage &message);

    static void globalShutdown();

public:
    static bool globalInit();

    static bool initInstance(SDL_Window *instance);

    static void shutdownInstance();

    static void preloadGame();

    static boost::shared_ptr<RBX::Game> getPreloadedGame();

    static void relinquishGame(boost::shared_ptr<RBX::Game> &game);
};

#endif //ROBLOX_CORE_H