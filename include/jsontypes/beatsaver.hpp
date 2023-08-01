#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_CLASS(BeatSaverUser,
    VALUE(std::string, name);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(BeatSaverPlaylist,
    VALUE(std::string, name);
    VALUE(BeatSaverUser, owner);
    VALUE(std::string, description);

    VALUE(std::string, downloadURL);
    VALUE(std::string, playlistImage);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(BeatSaverResponse,
    NAMED_VECTOR(BeatSaverPlaylist, Playlists, "docs");
    DISCARD_EXTRA_FIELDS
)

#include "apis.hpp"

struct BeatSaverPlaylistWrapper : Playlist {
    std::string Title() override {
        return playlist.name;
    }
    std::string Author() override {
        return playlist.owner.name;
    }
    std::string Description() override {
        return playlist.description;
    }
    std::string PlaylistURL() override {
        return playlist.downloadURL;
    }
    std::string ImageURL() override {
        return playlist.playlistImage;
    }
    BeatSaverPlaylistWrapper(BeatSaverPlaylist playlist) : playlist(playlist) {}
    private:
    BeatSaverPlaylist playlist;
};
