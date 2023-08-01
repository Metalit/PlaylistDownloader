#pragma once

#include <string>
#include "beatsaber-hook/shared/config/rapidjson-utils.hpp"

namespace WebUtils {
    void GetAsync(std::string url, std::function<void(long, std::string)> finished, std::function<void(float)> progressUpdate = nullptr);
    void GetAsync(std::string url, long timeout, std::function<void(long, std::string)> finished, std::function<void(float)> progressUpdate = nullptr);
}
