#pragma once

#include <string>
#include <functional>

struct Playlist {
    virtual std::string Title() = 0;
    virtual std::string Author() = 0;
    virtual std::string Description() = 0;
    virtual std::string PlaylistURL() = 0;
    virtual std::string ImageURL() = 0;
    virtual ~Playlist() = default;
};

void GetAccSaberPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search);
void GetBeatSaverPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search);
void GetHitbloqPlaylists(std::function<void(std::vector<std::unique_ptr<Playlist>> playlists)> callback, int page, std::string search);
