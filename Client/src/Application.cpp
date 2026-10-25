#include "Application.h"
#include "View.h"
#include "Functionmarshaller.h"

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "Script/ScriptContext.h"
#include "v8datamodel/ContentProvider.h"
#include "v8datamodel/Game.h"
#include "V8Xml/XmlSerializer.h"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

static po::variables_map vm;

namespace RBX
{
bool Application::initialize(SDL_Window* window)
{
    mainWindow = window;

    if (!Core::initInstance(window))
        return false;

    marshaller = RBX::FunctionMarshaller::GetWindow();

    initializeNewGame();
    return true;
}

void Application::shutdown()
{
    if (mainView)
    {
        mainView->Stop();
        Core::shutdownInstance();
    }
}

void Application::initializeNewGame()
{
    Core::preloadGame();

    mainView.reset(new RBX::View(mainWindow));
    mainView->Start(Core::getPreloadedGame());
}

bool Application::parseArguments(int argc, char* argv[])
{
    try
    {
        po::options_description desc("Options");
        desc.add_options()
            ("help,?", "Show this help message")
            ("authenticationUrl,a", po::value<std::string>(), "authentication url from server")
            ("authenticationTicket,t", po::value<std::string>(), "game session ticket from server")
            ("joinScriptUrl,j", po::value<std::string>(), "url of launch script from server");

        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

        if (vm.count("help") || argc < 2)
        {
            std::basic_stringstream<char> options;
            desc.print(options);
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Roblox", options.str().c_str(), nullptr);
            return false;
        }

        return true;
    }
    catch (std::exception& e)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Roblox", e.what(), nullptr);
        return false;
    }
}

bool Application::loadAppSettings()
{
    fs::path currentPath = fs::current_path();
    fs::path settingsPath = currentPath / "AppSettings.xml";

    if (!fs::exists(settingsPath))
    {
        SetBaseURL("https://www.roblox.com/");
        RBX::ContentProvider::setAssetFolder((currentPath / "content").string().c_str());
        return true;
    }

    const XmlTag& tagBaseUrl = RBX::Name::declare("BaseUrl");
    const XmlTag& tagContentFolder = RBX::Name::declare("ContentFolder");;

    std::ifstream fileStream(settingsPath.string());
    TextXmlParser parser(fileStream.rdbuf());
    std::unique_ptr<XmlElement> root;
    root = parser.parse();

    const XmlElement* xmlBaseUrl = root->findFirstChildByTag(tagBaseUrl);
    std::string valBaseUrl;
    if (xmlBaseUrl && xmlBaseUrl->getValue(valBaseUrl))
    {
        if (!valBaseUrl.empty() && (*valBaseUrl.rbegin()) != '/')
        {
            valBaseUrl.append("/");
        }
        SetBaseURL(valBaseUrl);
    }

    const XmlElement* xmlContentFolder = root->findFirstChildByTag(tagContentFolder);
    std::string valContentFolder;
    if (xmlContentFolder && xmlContentFolder->getValue(valContentFolder))
    {
        RBX::ContentProvider::setAssetFolder(valContentFolder.c_str());
    }

    return true;
}
}