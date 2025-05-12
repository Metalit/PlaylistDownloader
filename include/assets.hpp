#pragma once

#include "metacore/shared/assets.hpp"

#define DECLARE_ASSET(name, binary)       \
    const IncludedAsset name {            \
        Externs::_binary_##binary##_start, \
        Externs::_binary_##binary##_end    \
    };

#define DECLARE_ASSET_NS(namespaze, name, binary) \
    namespace namespaze { DECLARE_ASSET(name, binary) }

namespace IncludedAssets {
    namespace Externs {
        extern "C" uint8_t _binary_AccSaber_png_start[];
        extern "C" uint8_t _binary_AccSaber_png_end[];
        extern "C" uint8_t _binary_BeatLeader_png_start[];
        extern "C" uint8_t _binary_BeatLeader_png_end[];
        extern "C" uint8_t _binary_BeatSaver_png_start[];
        extern "C" uint8_t _binary_BeatSaver_png_end[];
        extern "C" uint8_t _binary_Hitbloq_png_start[];
        extern "C" uint8_t _binary_Hitbloq_png_end[];
        extern "C" uint8_t _binary_filtermenu_bsml_start[];
        extern "C" uint8_t _binary_filtermenu_bsml_end[];
        extern "C" uint8_t _binary_playlistdetail_bsml_start[];
        extern "C" uint8_t _binary_playlistdetail_bsml_end[];
        extern "C" uint8_t _binary_playlistlist_bsml_start[];
        extern "C" uint8_t _binary_playlistlist_bsml_end[];
    }

    // AccSaber.png
    DECLARE_ASSET(AccSaber_png, AccSaber_png);
    // BeatLeader.png
    DECLARE_ASSET(BeatLeader_png, BeatLeader_png);
    // BeatSaver.png
    DECLARE_ASSET(BeatSaver_png, BeatSaver_png);
    // Hitbloq.png
    DECLARE_ASSET(Hitbloq_png, Hitbloq_png);
    // filtermenu.bsml
    DECLARE_ASSET(filtermenu_bsml, filtermenu_bsml);
    // playlistdetail.bsml
    DECLARE_ASSET(playlistdetail_bsml, playlistdetail_bsml);
    // playlistlist.bsml
    DECLARE_ASSET(playlistlist_bsml, playlistlist_bsml);
}
