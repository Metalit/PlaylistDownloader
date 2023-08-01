#pragma once

#include "rapidjson-macros/shared/macros.hpp"

DECLARE_JSON_CLASS(AccSaberPlaylist,
    VALUE(std::string, displayName);
    VALUE(std::string, description);

    VALUE(std::string, downloadLink);
    VALUE(std::string, imageUrl);
    DISCARD_EXTRA_FIELDS
)

DECLARE_JSON_CLASS(AccSaberResponse,
    NAMED_VECTOR(AccSaberPlaylist, Playlists, rapidjson_macros_types::SelfValueType());
    DISCARD_EXTRA_FIELDS
)

#include "apis.hpp"

struct AccSaberPlaylistWrapper : Playlist {
    std::string Title() override {
        return playlist.displayName;
    }
    std::string Author() override {
        return "AccSaber";
    }
    std::string Description() override {
        return playlist.description;
    }
    std::string PlaylistURL() override {
        return playlist.downloadLink;
    }
    std::string ImageURL() override {
        return playlist.imageUrl;
    }
    AccSaberPlaylistWrapper(AccSaberPlaylist playlist) : playlist(playlist) {}
    private:
    AccSaberPlaylist playlist;
};
