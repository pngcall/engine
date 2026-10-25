#include "View.h"

#include "v8datamodel/DebugSettings.h"
#include "v8datamodel/GameBasicSettings.h"
#include "script/ScriptContext.h"
#include "FunctionMarshaller.h"
#include "rbx/Tasks/Coordinator.h"
#include "RenderJob.h"
#include "RenderSettingsItem.h"
#include "util/ScopedAssign.h"
#include "v8datamodel/Game.h"
#include "v8datamodel/UserController.h"
#include "format_string.h"

#include "util/RobloxGoogleAnalytics.h"
#include "rbx/SystemUtil.h"

LOGGROUP(PlayerShutdownLuaTimeoutSeconds)

LOGGROUP(RobloxWndInit)

FASTFLAGVARIABLE(GraphicsReportingInitErrorsToGAEnabled, true)

FASTFLAGVARIABLE(UseNewAppBridgeInputWindows, false)

DYNAMIC_FASTFLAGVARIABLE(FullscreenRefocusingFix, false)

namespace
{
SDL_Window* SetFocusWrapper(SDL_Window* hwnd)
{
    return hwnd;
}
} // namespace

namespace RBX
{
View::View(SDL_Window* h)
    : marshaller(nullptr)
{
    context.hWnd = h;
    marshaller = FunctionMarshaller::GetWindow();

    initializeView();
}

View::~View()
{
    RBXASSERT(!this->game && "Call Stop() before shutting down!");
    view.reset();

    if (marshaller)
        FunctionMarshaller::ReleaseWindow(marshaller);
}

void View::AboutToShutdown()
{
}

void View::initializeView()
{
    ViewBase::InitPluginModules();

    char* rgLogSuffix[5] = {};
    rgLogSuffix[static_cast<size_t>(CRenderSettings::OpenGL)] = "gfx_gl";

    std::vector<CRenderSettings::GraphicsMode> modes;

    RBX::CRenderSettings::GraphicsMode graphicsMode = CRenderSettingsItem::singleton().getLatchedGraphicsMode();
    modes.push_back(CRenderSettings::OpenGL);

    std::string lastMessage;
    bool success = false;
    size_t modei = 0;
    while (!success && modei < modes.size())
    {
        graphicsMode = modes[modei];
        try
        {
            view.reset(ViewBase::CreateView(graphicsMode, &context, &CRenderSettingsItem::singleton()));
            view->initResources();
            success = true;
        }
        catch (std::exception& e)
        {
            RBX::StandardOut::singleton()->printf(RBX::MESSAGE_WARNING, "Mode %d failed: \"%s\"", graphicsMode,
                e.what());
            lastMessage += e.what();
            lastMessage += " | ";

            modei++;
            if (modei < modes.size())
            {
                RBX::StandardOut::singleton()->printf(RBX::MESSAGE_WARNING, "Trying mode %d...", modes[modei]);
            }
        }
    }

    if (!success)
    {
        throw std::exception();
    }

    RBXASSERT(view);

    initializeSizes();
}

void View::resetScheduler()
{
    TaskScheduler& taskScheduler = TaskScheduler::singleton();
    taskScheduler.add(renderJob);
}

void View::initializeJobs()
{
    shared_ptr<DataModel> dataModel = game->getDataModel();
    renderJob.reset(new RenderJob(this, marshaller, dataModel));
}

void View::initializeInput()
{
}

void View::RemoveJobs()
{
    if (renderJob)
    {
        boost::function<void()> callback = [this]
        {
            marshaller->ProcessMessages();
        };
        TaskScheduler::singleton().removeBlocking(renderJob, callback);
    }

    // RenderJob is sure to be completed at this point, since removeBlocking returned - but it might have marshalled
    // renderPerform asynchronously before exiting, which means that we might still have a callback that uses this view
    // in the marshaller queue.
    // This makes sure that all pending marshalled events are processed to avoid a use after free.
    marshaller->ProcessMessages();

    // All render processing is complete; it's safe to reset job pointers now
    renderJob.reset();
}

shared_ptr<DataModel> View::getDataModel()
{
    return game ? game->getDataModel() : shared_ptr<DataModel>();
}

CRenderSettings::GraphicsMode View::GetLatchedGraphicsMode()
{
    return CRenderSettingsItem::singleton().getLatchedGraphicsMode();
}

void View::initializeSizes()
{
    if (GetHWnd() == nullptr)
    {
        return;
    }
    G3D::Vector2int16 currentDisplaySize = G3D::Vector2int16(800, 600);

    G3D::Vector2int16 fullscreenSize;

    RBX::CRenderSettings::ResolutionPreset preference = CRenderSettingsItem::singleton().getResolutionPreference();
    if (preference == RBX::CRenderSettings::ResolutionAuto)
    {
        fullscreenSize = currentDisplaySize;
    }
    else
    {
        const RBX::CRenderSettings::RESOLUTIONENTRY& res = CRenderSettingsItem::singleton().getResolutionPreset(
            preference);
        fullscreenSize.x = res.width;
        fullscreenSize.y = res.height;
    }

    // validate mode
    G3D::Vector2int16 windowSize = fullscreenSize;
    windowSize.x = std::min(static_cast<int>(windowSize.x), static_cast<int>(currentDisplaySize.x));
    windowSize.y = std::min(static_cast<int>(windowSize.y), static_cast<int>(currentDisplaySize.y));

    CRenderSettingsItem::singleton().setWindowSize(windowSize);
    CRenderSettingsItem::singleton().setFullscreenSize(fullscreenSize);
}

void View::onResize(int x, int y)
{
    view->onResize(x, y);
}

void View::unbindWorkspace()
{
    shared_ptr<DataModel> dm = getDataModel();
    DataModel::LegacyLock lock(dm, DataModelJob::Write);
    view->bindWorkspace(boost::shared_ptr<DataModel>());
}

void View::bindWorkspace()
{
    shared_ptr<DataModel> dm = getDataModel();
    DataModel::LegacyLock lock(dm, DataModelJob::Write);
    view->bindWorkspace(game->getDataModel());
    view->buildGui();
}

void View::Start(const shared_ptr<Game>& game)
{
    RBXASSERT(!this->game);
    this->game = game;

    bindWorkspace();
    initializeJobs();
    initializeInput(); // NOTE: have to do this here, Input requires datamodel access
    resetScheduler();
}

void View::Stop()
{
    RBXASSERT(this->game);
    this->RemoveJobs();

    if (game && game->getDataModel())
        if (auto* service = RBX::ServiceProvider::create<RBX::ControllerService>(game->getDataModel().get()))
            service->setHardwareDevice(nullptr);

    unbindWorkspace();

    game.reset();
}

void View::CloseWindow()
{
    SDL_DestroyWindow(static_cast<SDL_Window*>(context.hWnd));
}
}