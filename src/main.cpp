#include "main.hpp"
#include "customtypes/mainmenu.hpp"

#include "questui/shared/QuestUI.hpp"

static ModInfo modInfo;

Logger& getLogger() {
    static Logger* logger = new Logger(modInfo, {false, true});
    return *logger;
}

extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;

    getLogger().info("Completed setup!");
}

extern "C" void load() {
    il2cpp_functions::Init();

    QuestUI::Init();
    QuestUI::Register::RegisterMainMenuModSettingsViewController<PlaylistDownloader::MainMenu*>(modInfo, "More Playlists");
}
