#pragma once

#include "HMUI/ViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "bsml/shared/BSML/Components/CustomListTableData.hpp"
#include "custom-types/shared/macros.hpp"

DECLARE_CLASS_CODEGEN(PlaylistDownloader, PlaylistList, HMUI::ViewController,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_METHOD(void, SetupBSMLFields);
    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_STATIC_METHOD(PlaylistDownloader::PlaylistList*, GetInstance);
    DECLARE_INSTANCE_METHOD(void, Refresh, bool full);
    DECLARE_INSTANCE_METHOD(void, UpdateScrollView);
    DECLARE_INSTANCE_METHOD(void, SetLoading, bool value);

    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, loadingIndicator);
    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, noResultsText);
    DECLARE_INSTANCE_FIELD(BSML::CustomListTableData*, list);
    DECLARE_INSTANCE_FIELD(ListW<BSML::CustomCellInfo*>, playlistData);
    DECLARE_INSTANCE_METHOD(void, cellSelected, HMUI::TableView* table, int cellIdx);

    private:
    static inline PlaylistDownloader::PlaylistList* instance;
)
