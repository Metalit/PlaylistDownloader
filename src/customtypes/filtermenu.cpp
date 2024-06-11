#include "customtypes/filtermenu.hpp"

#include "HMUI/AnimatedSwitchView.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include "assets.hpp"
#include "bsml/shared/BSML.hpp"
#include "bsml/shared/Helpers/creation.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "config.hpp"
#include "main.hpp"
#include "manager.hpp"

DEFINE_TYPE(PlaylistDownloader, FilterMenu);

using namespace PlaylistDownloader;

void InstantSetToggle(UnityEngine::UI::Toggle* toggle, bool value) {
    if (toggle->m_IsOn == value)
        return;
    toggle->m_IsOn = value;
    auto animatedSwitch = toggle->GetComponent<HMUI::AnimatedSwitchView*>();
    animatedSwitch->HandleOnValueChanged(value);
    animatedSwitch->_switchAmount = value;
    animatedSwitch->LerpPosition(value);
    animatedSwitch->LerpColors(value, animatedSwitch->_highlightAmount, animatedSwitch->_disabledAmount);
}

void FilterMenu::OnEnable() {
    name = "PlaylistFilters";
    rectTransform->anchorMin = {0.5, 0.5};
    rectTransform->anchorMax = {0.5, 0.5};
    rectTransform->sizeDelta = {160, 15};
}

void FilterMenu::DidActivate(bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    if (firstActivation) {
        SetupBSMLFields();
        // AddHotReload(this, "filtermenu");
        BSML::parse_and_construct(IncludedAssets::filtermenu_bsml, transform, this);
    }
}

void FilterMenu::SetupBSMLFields() {
    using Item = HMUI::IconSegmentedControl::DataItem;
    sourceIconData = ListW<Item*>::New(4);
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(BeatSaver), "BeatSaver"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(Hitbloq), "Hitbloq"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(AccSaber), "AccSaber"));
    sourceIconData->Add(Item::New_ctor(PNG_SPRITE(BeatLeader), "BeatLeader"));
    sorts = ListW<StringW>::New(sortOptions.size());
    for (auto sort : sortOptions)
        sorts->Add(sort);
}

void FilterMenu::PostParse() {
    auto searchInput = BSML::Lite::CreateStringSetting(filterBar, "Search", "", {}, {0, -40, 0}, [this](StringW value) { stringInput(value); });
    searchInput->transform->SetSiblingIndex(1);
    sourceIconControl->_hideCellBackground = false;
    sourceIconControl->_overrideCellSize = true;
    sourceIconControl->_iconSize = 6;
    sourceIconControl->_padding = 2;
    sourceIconControl->ReloadData();
    Manager::SetSource(sourceIconControl->selectedCellNumber);
    Manager::SetSource(0);
    filterModal->Hide(false, nullptr);
    InstantSetToggle(curatedToggle->toggle, getConfig().curated.GetValue());
    InstantSetToggle(includeEmptyToggle->toggle, getConfig().includeEmpty.GetValue());
    for (int i = 0; i < sortOptions.size(); i++) {
        if (sortOptions[i] == getConfig().sort.GetValue()) {
            sortEnum->index = i;
            sortEnum->UpdateState();
        }
    }
}

void FilterMenu::OnDestroy() {
    instance = nullptr;
}

FilterMenu* FilterMenu::GetInstance() {
    if (!instance)
        instance = BSML::Helpers::CreateViewController<FilterMenu*>();
    return instance;
}

void FilterMenu::sourceSelected(HMUI::SegmentedControl* control, int cell) {
    logger.info("Source {} selected", cell);
    Manager::SetSource(cell);
    filterButton->gameObject->SetActive(SourceHasFilters(cell));
}

void FilterMenu::stringInput(StringW value) {
    logger.info("String \"{}\" input", value);
    Manager::SetSearch(value);
}

void FilterMenu::filterClicked() {
    logger.info("Filter button clicked");
    filterModal->Show(true, false, nullptr);
}

void FilterMenu::curatedToggled(bool value) {
    if (value == getConfig().curated.GetValue())
        return;
    getConfig().curated.SetValue(value);
    Manager::Refresh();
}

void FilterMenu::includeEmptyToggled(bool value) {
    if (value == getConfig().includeEmpty.GetValue())
        return;
    getConfig().includeEmpty.SetValue(value);
    Manager::Refresh();
}

void FilterMenu::sortPicked(StringW value) {
    if (value == getConfig().sort.GetValue())
        return;
    getConfig().sort.SetValue(value);
    Manager::Refresh();
}
