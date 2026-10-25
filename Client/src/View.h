#ifndef ROBLOX_VIEW_H
#define ROBLOX_VIEW_H

#include <boost/smart_ptr/scoped_ptr.hpp>
#include <SDL.h>


#include "GfxBase/ViewBase.h"

namespace RBX
{
    // Forward declarations
    class FunctionMarshaller;
    class Game;
    struct OSContext;
    class RenderJob;
    class ViewBase;

    namespace Tasks
    {
        class Sequence;
    }

    // Class responsible for the game view
    class View
    {
    public:
        View(SDL_Window* h);

        ~View();

        void AboutToShutdown();

        void Start(const boost::shared_ptr<Game>& game);

        void Stop();

        void ShowWindow();

        void CloseWindow();

        SDL_Window* GetHWnd() const
        {
            return static_cast<SDL_Window*>(context.hWnd);
        }

        // TODO: refactor verbs so this isn't needed
        ViewBase* GetGfxView() const
        {
            return view.get();
        }

        CRenderSettings::GraphicsMode GetLatchedGraphicsMode();

        boost::shared_ptr<DataModel> getDataModel();

        void onResize(int x, int y);
    private:
        // View references, but doesn't own the game
        boost::shared_ptr<Game> game;

        // The OS context used by the view.
        OSContext context;

        // The view into the game world.
        boost::scoped_ptr<RBX::ViewBase> view;

        boost::shared_ptr<Tasks::Sequence> sequence;
        boost::shared_ptr<RenderJob> renderJob;
        FunctionMarshaller* marshaller;


        void initializeSizes();

        void bindWorkspace();

        void unbindWorkspace();

        void initializeView();

        void initializeInput();

        void resetScheduler();

        void initializeJobs();

        void RemoveJobs();
    };
} // namespace RBX

#endif //ROBLOX_VIEW_H