#pragma once

#include "custom-types/shared/macros.hpp"

#include "HMUI/ViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "HMUI/ImageView.hpp"
#include "TMPro/TextMeshProUGUI.hpp"
#include "HMUI/TextPageScrollView.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "bsml/shared/BSML/Components/CustomListTableData.hpp"
#include "questui/shared/CustomTypes/Components/ProgressBar/ProgressBar.hpp"

#define METHOD(...) il2cpp_utils::il2cpp_type_check::MetadataGetter<&__VA_ARGS__>::get()

DECLARE_CLASS_CODEGEN(PlaylistDownloader, PlaylistDetail, HMUI::ViewController,

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_OVERRIDE_METHOD(void, DidActivate, METHOD(HMUI::ViewController::DidActivate), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_METHOD(void, SetupBSMLFields);
    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_STATIC_METHOD(PlaylistDownloader::PlaylistDetail*, GetInstance);
    DECLARE_INSTANCE_METHOD(void, Refresh, bool full);
    DECLARE_INSTANCE_METHOD(void, UpdateScrollView);
    DECLARE_INSTANCE_METHOD(void, SetLoading, bool value);

    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, loadingIndicator);
    DECLARE_INSTANCE_FIELD(HMUI::ImageView*, cover);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, name);
    DECLARE_INSTANCE_FIELD(TMPro::TextMeshProUGUI*, author);
    DECLARE_INSTANCE_FIELD(HMUI::TextPageScrollView*, description);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, download);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, downloadSongs);
    DECLARE_INSTANCE_FIELD(BSML::CustomListTableData*, list);
    DECLARE_INSTANCE_FIELD(List<BSML::CustomCellInfo*>*, songData);
    DECLARE_INSTANCE_METHOD(void, downloadClicked);
    DECLARE_INSTANCE_METHOD(void, downloadSongsClicked);

    DECLARE_INSTANCE_FIELD(QuestUI::ProgressBar*, downloadProgress);

    private:
    static inline PlaylistDownloader::PlaylistDetail* instance;
)
