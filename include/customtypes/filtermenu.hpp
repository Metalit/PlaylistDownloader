#pragma once

#include "custom-types/shared/macros.hpp"

#include "HMUI/ViewController.hpp"
#include "HMUI/IconSegmentedControl.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Button.hpp"
#include "HMUI/ModalView.hpp"
#include "bsml/shared/BSML/Components/Settings/ToggleSetting.hpp"
#include "bsml/shared/BSML/Components/Settings/ListSetting.hpp"

DECLARE_CLASS_CODEGEN(PlaylistDownloader, FilterMenu, HMUI::ViewController,
    DECLARE_DEFAULT_CTOR();

    DECLARE_INSTANCE_METHOD(void, OnEnable);
    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_METHOD(void, SetupBSMLFields);
    DECLARE_INSTANCE_METHOD(void, PostParse);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_STATIC_METHOD(PlaylistDownloader::FilterMenu*, GetInstance);

    DECLARE_INSTANCE_FIELD(ListW<HMUI::IconSegmentedControl::DataItem*>, sourceIconData);
    DECLARE_INSTANCE_FIELD(ListW<StringW>, sorts);

    DECLARE_INSTANCE_FIELD(UnityEngine::Transform*, filterBar);
    DECLARE_INSTANCE_FIELD(HMUI::IconSegmentedControl*, sourceIconControl);
    DECLARE_INSTANCE_FIELD(UnityEngine::UI::Button*, filterButton);
    DECLARE_INSTANCE_FIELD(HMUI::ModalView*, filterModal);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, curatedToggle);
    DECLARE_INSTANCE_FIELD(BSML::ToggleSetting*, includeEmptyToggle);
    DECLARE_INSTANCE_FIELD(BSML::ListSetting*, sortEnum);
    DECLARE_INSTANCE_METHOD(void, sourceSelected, HMUI::SegmentedControl* control, int cell);
    DECLARE_INSTANCE_METHOD(void, stringInput, StringW value);
    DECLARE_INSTANCE_METHOD(void, filterClicked);
    DECLARE_INSTANCE_METHOD(void, curatedToggled, bool value);
    DECLARE_INSTANCE_METHOD(void, includeEmptyToggled, bool value);
    DECLARE_INSTANCE_METHOD(void, sortPicked, StringW value);

    private:
    static inline PlaylistDownloader::FilterMenu* instance;
)
