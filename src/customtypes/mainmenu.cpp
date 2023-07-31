#include "main.hpp"
#include "customtypes/mainmenu.hpp"
#include "customtypes/filtermenu.hpp"
#include "customtypes/playlistlist.hpp"
#include "customtypes/playlistdetail.hpp"

#include "questui/shared/BeatSaberUI.hpp"

DEFINE_TYPE(PlaylistDownloader, MainMenu);

using namespace PlaylistDownloader;

#include "GlobalNamespace/SelectLevelCategoryViewController.hpp"

void MainMenu::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    set_name("PlaylistDownloaderMenu");
    orientation = Orientation::Vertical;
    viewControllersSeparator = -5;
    SetChildViewControllers({(HMUI::ViewController*)FilterMenu::GetInstance(), (HMUI::ViewController*)GetSubNavigationController()});
    GetSubNavigationController()->get_transform()->SetAsFirstSibling();
    // GetSubNavigationController()->SetChildViewController((HMUI::ViewController*)PlaylistList::GetInstance());
    GetSubNavigationController()->SetChildViewControllers({(HMUI::ViewController*)PlaylistList::GetInstance(), (HMUI::ViewController*)PlaylistDetail::GetInstance()});
}

HMUI::NavigationController* MainMenu::GetSubNavigationController() {
    if (!instance) {
        instance = QuestUI::BeatSaberUI::CreateViewController<HMUI::NavigationController*>();
        instance->set_name("HorizontalNavigationController");
    }
    return instance;
}
