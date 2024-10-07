#include "apis.hpp"

#include "config.hpp"
#include "jsontypes/accsaber.hpp"
#include "jsontypes/beatleader.hpp"
#include "jsontypes/beatsaver.hpp"
#include "jsontypes/hitbloq.hpp"
#include "main.hpp"
#include "web-utils/shared/WebUtils.hpp"

inline std::string lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

template <class T>
void GetAsync(WebUtils::URLOptions url, std::function<void(T)> onResponse) {
    WebUtils::GetAsync<WebUtils::StringResponse>(url, [onResponse = std::move(onResponse), url = url.fullURl()](WebUtils::StringResponse response) {
        if (!response.responseData || !response.IsSuccessful())
            logger.error("{} request failed: {} {}", url, response.httpCode, response.curlStatus);
        T ret;
        try {
            ReadFromString(*response.responseData, ret);
        } catch (JSONException const& exc) {
            logger.error("{} deserialize failed \"{}\": {}", url, *response.responseData, exc.what());
        }
        onResponse(ret);
    });
}

void GetAccSaberPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search) {
    if (page > 0)
        return;
    GetAsync<AccSaberResponse>({"https://api.accsaber.com/playlists"}, [search = lower(search), callback](auto response) {
        std::vector<std::unique_ptr<Playlist>> ret = {};
        for (auto& playlist : response.Playlists) {
            if (search.empty() || lower(playlist.displayName).find(search) != std::string::npos)
                ret.emplace_back(FromAccSaber(playlist));
        }
        callback(std::move(ret));
    });
}

void GetBeatSaverPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search) {
    std::string url = "https://api.beatsaver.com/playlists/search/";
    url += std::to_string(page);
    std::unordered_map<std::string, std::string> queries;
    if (getConfig().curated.GetValue())
        queries["curated"] = "true";
    queries["includeEmpty"] = getConfig().includeEmpty.GetValue() ? "true" : "false";
    queries["sortOrder"] = getConfig().sort.GetValue();
    if (!search.empty())
        queries["q"] = search;
    GetAsync<BeatSaverResponse>({url, queries}, [callback](auto response) {
        std::vector<std::unique_ptr<Playlist>> ret = {};
        for (auto& playlist : response.Playlists)
            ret.emplace_back(FromBeatSaver(playlist));
        callback(std::move(ret));
    });
}

void GetBeatLeaderPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search) {
    std::string url = "https://api.beatleader.xyz/events";
    std::unordered_map<std::string, std::string> queries;
    queries["page"] = std::to_string(page + 1);
    if (!search.empty())
        queries["search"] = search;
    GetAsync<BeatLeaderResponse>({url, queries}, [callback](auto response) {
        std::vector<std::unique_ptr<Playlist>> ret = {};
        for (auto& playlist : response.Playlists) {
            if (playlist.downloadable)
                ret.emplace_back(FromBeatLeader(playlist));
        }
        callback(std::move(ret));
    });
}

void GetHitbloqPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search) {
    if (page > 0)
        return;
    std::string url = "https://hitbloq.com/api/map_pools_detailed";
    std::unordered_map<std::string, std::string> queries;
    if (!search.empty())
        queries["search"] = search;
    GetAsync<HitbloqResponse>({url, queries}, [callback](auto response) {
        std::vector<std::unique_ptr<Playlist>> ret = {};
        for (auto& playlist : response.Playlists)
            ret.emplace_back(FromHitbloq(playlist));
        callback(std::move(ret));
    });
}
