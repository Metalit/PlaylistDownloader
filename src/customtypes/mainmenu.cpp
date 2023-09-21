#include "main.hpp"
#include "manager.hpp"
#include "customtypes/mainmenu.hpp"
#include "customtypes/filtermenu.hpp"
#include "customtypes/playlistlist.hpp"
#include "customtypes/playlistdetail.hpp"

#include "questui/shared/BeatSaberUI.hpp"

DEFINE_TYPE(PlaylistDownloader, MainMenu);

using namespace PlaylistDownloader;

void MainMenu::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    instance = this;
    if (firstActivation) {
        set_name("PlaylistDownloaderMenu");
        orientation = Orientation::Vertical;
        viewControllersSeparator = -5;
    }
    SetChildViewControllers({(HMUI::ViewController*)FilterMenu::GetInstance(), (HMUI::ViewController*)GetSubNavigationController()});
    GetSubNavigationController()->get_transform()->SetAsFirstSibling();
    if (detailShown)
        GetSubNavigationController()->SetChildViewControllers({(HMUI::ViewController*)PlaylistList::GetInstance(), (HMUI::ViewController*)PlaylistDetail::GetInstance()});
    else
        GetSubNavigationController()->SetChildViewController((HMUI::ViewController*)PlaylistList::GetInstance());
}

HMUI::NavigationController* MainMenu::GetSubNavigationController() {
    if (!subNavigationController) {
        subNavigationController = QuestUI::BeatSaberUI::CreateViewController<HMUI::NavigationController*>();
        subNavigationController->set_name("HorizontalNavigationController");
    }
    return subNavigationController;
}

void MainMenu::OnDestroy() {
    Manager::Invalidate();
    instance = nullptr;
}

void MainMenu::SetDetailShown(bool value) {
    if (!instance)
        return;
    instance->detailShown = value;
    auto controller = instance->GetSubNavigationController();
    if (!controller->get_isInViewControllerHierarchy())
        return;
    int count = controller->viewControllers->get_Count();
    if (value && count == 1)
        controller->PushViewController((HMUI::ViewController*)PlaylistDetail::GetInstance(), nullptr, false);
    else if (!value && count == 2)
        controller->PopViewController(nullptr, true);
}
