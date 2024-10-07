#pragma once

#include <functional>
#include <string>

#include "assets.hpp"

struct Playlist {
    std::string Title;
    std::string Author;
    std::string Description;
    std::string PlaylistURL;
    std::string ImageURL;
    IncludedAsset const* FallbackImage;
};

void GetAccSaberPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search);
void GetBeatSaverPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search);
void GetBeatLeaderPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search);
void GetHitbloqPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search);
