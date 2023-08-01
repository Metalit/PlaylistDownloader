#pragma once

#include "custom-types/shared/macros.hpp"

#include "HMUI/ViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "bsml/shared/BSML/Components/CustomListTableData.hpp"

#define METHOD(...) il2cpp_utils::il2cpp_type_check::MetadataGetter<&__VA_ARGS__>::get()

DECLARE_CLASS_CODEGEN(PlaylistDownloader, PlaylistList, HMUI::ViewController,

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_OVERRIDE_METHOD(void, DidActivate, METHOD(HMUI::ViewController::DidActivate), bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_METHOD(void, SetupBSMLFields);
    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_DTOR(dtor);
    DECLARE_STATIC_METHOD(PlaylistDownloader::PlaylistList*, GetInstance);
    DECLARE_INSTANCE_METHOD(void, Refresh);
    DECLARE_INSTANCE_METHOD(void, UpdateScrollView);
    DECLARE_INSTANCE_METHOD(void, SetLoading, bool value);

    DECLARE_INSTANCE_FIELD(UnityEngine::GameObject*, loadingIndicator);
    DECLARE_INSTANCE_FIELD(BSML::CustomListTableData*, list);
    DECLARE_INSTANCE_FIELD(List<BSML::CustomCellInfo*>*, playlistData);
    DECLARE_INSTANCE_METHOD(void, cellSelected, HMUI::TableView* table, int cellIdx);

    private:
    static inline PlaylistDownloader::PlaylistList* instance;
)
