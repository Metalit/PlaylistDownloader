#include "main.hpp"

#include "bsml/shared/BSML.hpp"
#include "config.hpp"
#include "customtypes/mainmenu.hpp"
#include "scotland2/shared/modloader.h"

static modloader::ModInfo modInfo = {MOD_ID, VERSION, 0};

extern "C" void setup(CModInfo* info) {
    Paper::Logger::RegisterFileContextId(MOD_ID);

    info->id = MOD_ID;
    info->version = VERSION;
    modInfo.assign(*info);

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
