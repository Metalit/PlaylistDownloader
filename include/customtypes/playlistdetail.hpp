#pragma once

#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "HMUI/ImageView.hpp"
#include "HMUI/TextPageScrollView.hpp"
#include "HMUI/ViewController.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "apis.hpp"
#include "bsml/shared/BSML/Components/CustomListTableData.hpp"
#include "bsml/shared/BSML/Components/ProgressBar.hpp"
#include "custom-types/shared/macros.hpp"
#include "playlistcore/shared/PlaylistCore.hpp"

DECLARE_CLASS_CODEGEN(PlaylistDownloader, PlaylistDetail, HMUI::ViewController,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_INSTANCE_METHOD(void, OnDisable);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_METHOD(void, SetupBSMLFields);
    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_STATIC_METHOD(PlaylistDownloader::PlaylistDetail*, GetInstance);
    DECLARE_INSTANCE_METHOD(void, Refresh, bool full);
    DECLARE_INSTANCE_METHOD(void, UpdateScrollView);
    DECLARE_INSTANCE_METHOD(void, UpdateDownloadButtons);
    DECLARE_INSTANCE_METHOD(void, SetLoading, bool value);
    DECLARE_INSTANCE_METHOD(void, SetDownloading, bool value);
    DECLARE_INSTANCE_METHOD(void, AddDownload, bool update, bool songs);
    DECLARE_INSTANCE_METHOD(void, PopDownload);

    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, loadingIndicator);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, noResultsText);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, cover);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, name);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, author);
    DECLARE_INSTANCE_FIELD(HMUI::TextPageScrollView*, description);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, download);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, downloadSongs);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, update);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, updateSongs);
    DECLARE_INSTANCE_FIELD(BSML::CustomListTableData*, list);
    DECLARE_INSTANCE_FIELD(ListW<BSML::CustomCellInfo*>, songData);
    DECLARE_INSTANCE_METHOD(void, downloadClicked);
    DECLARE_INSTANCE_METHOD(void, downloadSongsClicked);
    DECLARE_INSTANCE_METHOD(void, updateClicked);
    DECLARE_INSTANCE_METHOD(void, updateSongsClicked);
    DECLARE_INSTANCE_METHOD(void, songClicked, UnityW<HMUI::TableView>, int index);

    DECLARE_INSTANCE_FIELD(BSML::ProgressBar*, downloadProgress);

    DECLARE_INSTANCE_FIELD(GlobalNamespace::SongPreviewPlayer*, previewer);
    DECLARE_INSTANCE_FIELD(UnityEngine::AudioClip*, audio);

    private:
    static inline PlaylistDownloader::PlaylistDetail* instance;

    struct DownloadInfo {
        Playlist playlist;
        bool update;
        bool songs;
    };
    std::vector<DownloadInfo> queue = {};
    std::string currentDownloadUrl = "";

    void DownloadMissingSongs(PlaylistCore::Playlist* playlist);
    custom_types::Helpers::Coroutine PlayPreview(int index);
)
