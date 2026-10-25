#include "ContextGL.h"

#include "HeadersGL.h"

#include "rbx/Debug.h"

#include <SDL2/SDL.h>

LOGGROUP (Graphics)

namespace RBX {
    namespace Graphics {
        class ContextGLSDL2 : public ContextGL {
        public:
            ContextGLSDL2(void *windowHandle) {
                aNativeWindow = static_cast<SDL_Window *>(windowHandle);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
                context = SDL_GL_CreateContext(aNativeWindow);
                SDL_GL_MakeCurrent(aNativeWindow, context);

                glewExperimental = true;
                glewInitRBX();
            }

            virtual void setCurrent() {
                SDL_GL_MakeCurrent(aNativeWindow, context);
            }

            virtual void swapBuffers() {
                SDL_GL_SwapWindow(aNativeWindow);
            }

            virtual unsigned int getMainFramebufferId() {
                return 0;
            }

            virtual bool isMainFramebufferRetina() {
                return false;
            }

            virtual std::pair<unsigned int, unsigned int>
            updateMainFramebuffer(unsigned int width, unsigned int height) {
                int w, h;
                SDL_GetWindowSize(aNativeWindow, &w, &h);

                return std::make_pair(w, h);
            }

        private:
            SDL_Window *aNativeWindow;
            SDL_GLContext context;
        };

        ContextGL *ContextGL::create(void *windowHandle) {
            return new ContextGLSDL2(windowHandle);
        }
    } // namespace Graphics
} // namespace RBX