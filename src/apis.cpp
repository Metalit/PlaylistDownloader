#include "main.hpp"
#include "apis.hpp"
#include "webutil.hpp"

#include "jsontypes/accsaber.hpp"
#include "jsontypes/beatsaver.hpp"
#include "jsontypes/beatleader.hpp"
#include "jsontypes/hitbloq.hpp"

inline std::string lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

void GetAccSaberPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search) {
    if (page > 0)
        return;
    WebUtils::GetAsync("https://api.accsaber.com/playlists", [search = lower(search), callback](long httpCode, std::string data) {
        AccSaberResponse resp;
        try {
            ReadFromString(data, resp);
        } catch (const JSONException& exc) {
            getLogger().error("Failed to deserialize accsaber response %s: %s", data.c_str(), exc.what());
        }
        std::vector<std::unique_ptr<Playlist>> ret = {};
        for (auto& playlist : resp.Playlists) {
            if (search.empty() || lower(playlist.displayName).find(search) != std::string::npos)
                ret.emplace_back(std::make_unique<AccSaberPlaylistWrapper>(playlist));
        }
        callback(std::move(ret));
    });
}

void GetBeatSaverPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search) {
    std::string url = "https://api.beatsaver.com/playlists/search/";
    url += std::to_string(page) + "?sortOrder=Latest";
    if (!search.empty())
        url += "&q=" + search;
    WebUtils::GetAsync(url, [callback](long httpCode, std::string data) {
        BeatSaverResponse resp;
        try {
            ReadFromString(data, resp);
        } catch (const JSONException& exc) {
            getLogger().error("Failed to deserialize beatsaver response %s: %s", data.c_str(), exc.what());
        }
        std::vector<std::unique_ptr<Playlist>> ret = {};
        for (auto& playlist : resp.Playlists)
            ret.emplace_back(std::make_unique<BeatSaverPlaylistWrapper>(playlist));
        callback(std::move(ret));
    });
}

void GetBeatLeaderPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search) {
    std::string url = "https://api.beatleader.xyz/events";
    url += "?page=" + std::to_string(page + 1);
    if (!search.empty())
        url += "&search=" + search;
    WebUtils::GetAsync(url, [callback](long httpCode, std::string data) {
        BeatLeaderResponse resp;
        try {
            ReadFromString(data, resp);
        } catch (const JSONException& exc) {
            getLogger().error("Failed to deserialize beatleader response %s: %s", data.c_str(), exc.what());
        }
        std::vector<std::unique_ptr<Playlist>> ret = {};
        for (auto& playlist : resp.Playlists)
            ret.emplace_back(std::make_unique<BeatLeaderPlaylistWrapper>(playlist));
        callback(std::move(ret));
    });
}

void GetHitbloqPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search) {
    if (page > 0)
        return;
    std::string url = "https://hitbloq.com/api/map_pools_detailed";
    if (!search.empty())
        url += "?search=" + search;
    WebUtils::GetAsync(url, [callback](long httpCode, std::string data) {
        HitbloqResponse resp;
        try {
            ReadFromString(data, resp);
        } catch (const JSONException& exc) {
            getLogger().error("Failed to deserialize hitbloq response %s: %s", data.c_str(), exc.what());
        }
        std::vector<std::unique_ptr<Playlist>> ret = {};
        for (auto& playlist : resp.Playlists)
            ret.emplace_back(std::make_unique<HitbloqPlaylistWrapper>(playlist));
        callback(std::move(ret));
    });
}
