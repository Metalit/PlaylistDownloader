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
    GetSubNavigationController()->SetChildViewController((HMUI::ViewController*)PlaylistList::GetInstance());
    instance = this;
}

HMUI::NavigationController* MainMenu::GetSubNavigationController() {
    if (!subNavigationController) {
        subNavigationController = QuestUI::BeatSaberUI::CreateViewController<HMUI::NavigationController*>();
        subNavigationController->set_name("HorizontalNavigationController");
    }
    return subNavigationController;
}

void MainMenu::dtor() {
    instance = nullptr;
}

void MainMenu::SetDetailShown(bool value) {
    if (!instance)
        return;
    auto controller = instance->GetSubNavigationController();
    int count = controller->viewControllers->get_Count();
    if (value && count == 1)
        controller->PushViewController((HMUI::ViewController*)PlaylistDetail::GetInstance(), nullptr, false);
    else if (!value && count == 2)
        controller->PopViewController(nullptr, true);
}
