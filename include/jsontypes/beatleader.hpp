#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_CLASS(BeatLeaderPlaylist,
    VALUE(std::string, name);

    VALUE(int, playlistId);
    VALUE(std::string, image);
    VALUE(bool, downloadable);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(BeatLeaderResponse,
    NAMED_VECTOR(BeatLeaderPlaylist, Playlists, "data");
    DISCARD_EXTRA_FIELDS
)

#include "apis.hpp"

struct BeatLeaderPlaylistWrapper : Playlist {
    std::string Title() override {
        return playlist.name;
    }
    std::string Author() override {
        return "BeatLeader";
    }
    std::string Description() override {
        return "";
    }
    std::string PlaylistURL() override {
        return "https://api.beatleader.xyz/playlist/" + std::to_string(playlist.playlistId);
    }
    std::string ImageURL() override {
        return playlist.image;
    }
    BeatLeaderPlaylistWrapper(BeatLeaderPlaylist playlist) : playlist(playlist) {}
    private:
    BeatLeaderPlaylist playlist;
};
