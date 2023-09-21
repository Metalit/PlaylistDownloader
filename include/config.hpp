#pragma once

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(Config,
    CONFIG_VALUE(curated, bool, "Curated", false)
    CONFIG_VALUE(includeEmpty, bool, "Include Empty", false)
    CONFIG_VALUE(sort, std::string, "Sort Order", "Latest")
)

inline const std::vector<std::string> sortOptions = {
    "Latest",
    "Relevance",
    // these don't do anything?
    // "Rating",
    // "Curated",
};

inline bool SourceHasFilters(int source) {
    return source == 0;
}
