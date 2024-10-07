#pragma once

#include "custom-types/shared/macros.hpp"

#include "HMUI/NavigationController.hpp"

DECLARE_CLASS_CODEGEN(PlaylistDownloader, MainMenu, HMUI::NavigationController,
    DECLARE_DEFAULT_CTOR();

    DECLARE_OVERRIDE_METHOD_MATCH(void, DidActivate, &HMUI::ViewController::DidActivate, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
    DECLARE_INSTANCE_METHOD(HMUI::NavigationController*, GetSubNavigationController);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);

    DECLARE_STATIC_METHOD(void, SetDetailShown, bool value);
    DECLARE_INSTANCE_FIELD_PRIVATE(bool, detailShown);

    private:
    HMUI::NavigationController* subNavigationController;
    static inline PlaylistDownloader::MainMenu* instance;
)
