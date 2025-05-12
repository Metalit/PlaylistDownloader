#include "main.hpp"

#include "bsml/shared/BSML.hpp"
#include "config.hpp"
#include "customtypes/mainmenu.hpp"
#include "scotland2/shared/modloader.h"

static modloader::ModInfo modInfo = {MOD_ID, VERSION, 0};

extern "C" void setup(CModInfo* info) {
    *info = modInfo.to_c();
    Paper::Logger::RegisterFileContextId(MOD_ID);
    getConfig().Init(modInfo);
    logger.info("PlaylistDownloader setup!");
}

extern "C" void late_load() {
    il2cpp_functions::Init();
    custom_types::Register::AutoRegister();
    BSML::Init();

    BSML::Register::RegisterMainMenu<PlaylistDownloader::MainMenu*>("Playlist Downloader", "More Playlists", "Download more playlists!");

    logger.info("PlaylistDownloader loaded!");
}
