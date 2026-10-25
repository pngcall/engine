#include "Application.h"
#include "Core.h"
#include "FunctionMarshaller.h"

#include "v8datamodel/FastLogSettings.h";

int main(const int argc, char* argv[])
{
    RBX::Application app;
    if (!RBX::Application::loadAppSettings() || !RBX::Application::parseArguments(argc, argv))
        return -1;

    std::string clientSettingsString;
    FetchClientSettingsData(CLIENT_APP_SETTINGS_STRING, CLIENT_SETTINGS_API_KEY, &clientSettingsString);
    LoadClientSettingsFromString(CLIENT_APP_SETTINGS_STRING, clientSettingsString,
        &RBX::ClientAppSettings::singleton());

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Roblox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!app.initialize(window))
        return -1;

    SDL_ShowCursor(SDL_DISABLE);
    SDL_Event event;
    bool isRunning = true;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    app.shutdown();
                    isRunning = false;
                    break;
                case SDL_USEREVENT:
                    switch (event.user.code)
                    {
                        case 0xc10554e: // closure
                            RBX::FunctionMarshaller::handleAppEvent(event.user.data1);
                            break;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return 0;
}