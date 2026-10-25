#include "Core.h"

#include "MachineConfiguration.h"
#include "rbx/Profiler.h"
#include "Util/MD5Hasher.h"
#include "V8DataModel/ContentProvider.h"
#include "V8DataModel/DataModel.h"
#include "V8DataModel/DebugSettings.h"
#include "v8datamodel/Game.h"
#include "script/ScriptContext.h"

FASTFLAG(GoogleAnalyticsTrackingEnabled)

DYNAMIC_LOGGROUP(GoogleAnalyticsTracking)

LOGGROUP(Network)

LOGGROUP(PlayerShutdownLuaTimeoutSeconds)

SDL_Window* Core::theInstance = nullptr;
bool Core::initialized = false;
static rbx::signals::scoped_connection messageOutConnection;

static boost::shared_ptr<RBX::Game> preloadedGame;
static boost::mutex preloadedGameMutex;
static bool preloadShuttingDown = false;

static boost::thread releaseGameThread;

static void do_preloadGame()
{
    // Avoid having 2 DataModels open at once
    // It should be safe, but may as well not play with fire
    releaseGameThread.join();

    boost::mutex::scoped_lock lock(preloadedGameMutex);
    if (preloadShuttingDown)
        return;
    if (!preloadedGame)
    {
        RBX::Time start = RBX::Time::now<RBX::Time::Fast>();
        preloadedGame.reset(new RBX::SecurePlayerGame(nullptr, ::GetBaseURL().c_str()));
        RBX::Time stop = RBX::Time::now<RBX::Time::Fast>();
        double secs = (stop - start).seconds();
        RBX::StandardOut::singleton()->printf(RBX::MESSAGE_OUTPUT, "Preloaded Game %gsec", secs);
    }
    else
        RBX::StandardOut::singleton()->printf(RBX::MESSAGE_OUTPUT, "Already preloaded Game");
}

void Core::preloadGame()
{
    RBX::StandardOut::singleton()->printf(RBX::MESSAGE_OUTPUT, "Requesting preload Game");
    boost::thread(
        []
        {
            do_preloadGame();
        });
}


bool Core::initInstance(SDL_Window* instance)
{
    theInstance = instance;

    if (globalInit())
    {
        return true;
    }

    return false;
}

void Core::shutdownInstance()
{
    globalShutdown();
    theInstance = nullptr;
}

bool Core::globalInit()
{
    if (initialized)
    {
        return true;
    }
    RBX::Game::globalInit(false);

    if (RBX::DataModel::hash.empty())
    {
        RBX::DataModel::hash = RBX::CollectMd5Hash("./Player");
    }

    FLog::ResetSynchronizedVariablesState();
    RBX::Http::SetUseStatistics(true);

    RBX::postMachineConfiguration(::GetBaseURL().c_str(), 0);

    messageOutConnection = RBX::StandardOut::singleton()->messageOut.connect(&onMessageOut);

    RBX::GlobalAdvancedSettings::singleton()->loadState("");
    {
        RBX::Security::Impersonator impersonate(RBX::Security::RobloxGameScript_);
        RBX::GlobalBasicSettings::singleton()->loadState("");
    }

    RBX::Profiler::onThreadCreate("Main");

    // Initialize the TaskScheduler (after loading configs)
    RBX::TaskScheduler::singleton().setThreadCount(RBX::TaskSchedulerSettings::singleton().getThreadPoolConfig());

    initialized = true;

    return true;
}


boost::shared_ptr<RBX::Game> Core::getPreloadedGame() {
    boost::mutex::scoped_lock lock(preloadedGameMutex);
    if (!preloadedGame) {
        RBX::Time start = RBX::Time::now<RBX::Time::Fast>();
        preloadedGame.reset(new RBX::SecurePlayerGame(nullptr, ::GetBaseURL().c_str()));

        if (FLog::PlayerShutdownLuaTimeoutSeconds > 0) {
            (void)preloadedGame->getDataModel()->create<RBX::ScriptContext>();
        }

        RBX::Time stop = RBX::Time::now<RBX::Time::Fast>();
        double secs = (stop - start).seconds();
        RBX::StandardOut::singleton()->printf(RBX::MESSAGE_OUTPUT, "Loaded Game %gsec", secs);
    }

    return preloadedGame;
}

static void releaseGame(const boost::shared_ptr<RBX::Game>& game)
{
    RBX::GlobalBasicSettings::singleton()->saveState();
    game->shutdown();
}

void Core::relinquishGame(boost::shared_ptr<RBX::Game>& game)
{
    releaseGameThread.join();

    // Defer deletion of DataModel for later (so that we don't block)
    releaseGameThread = boost::thread(
        [game]
        {
            releaseGame(game);
        });

    game.reset();
}

void Core::globalShutdown()
{
    {
        boost::mutex::scoped_lock lock(preloadedGameMutex);
        preloadShuttingDown = true;
        if (preloadedGame)
            preloadedGame->shutdown(); // Don't rely on destructor, in case somebody else holds a reference
        preloadedGame.reset();
    }

    // Don't exit until Game has been shut down. Otherwise we might get crashes in static destructors
    releaseGameThread.join();

    if (!initialized)
        return;

    RBX::Game::globalExit();
    messageOutConnection.disconnect();
}

void Core::onMessageOut(const RBX::StandardOutMessage& message)
{
    switch (message.type)
    {
    case RBX::MESSAGE_INFO:
        std::cout << "INFO: " << message.message << std::endl;
        break;
    case RBX::MESSAGE_WARNING:
        std::cout << "WARNING: " << message.message << std::endl;
        break;
    case RBX::MESSAGE_ERROR:
        std::cerr << "ERROR: " << message.message << std::endl;
        break;
    default:
        std::cout << message.message << std::endl;
        break;
    }
}