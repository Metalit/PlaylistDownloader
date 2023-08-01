#pragma once

#include "apis.hpp"
#include "songdownloader/shared/BeatSaverAPI.hpp"
#include "playlistcore/shared/Types/BPList.hpp"

#include "UnityEngine/Sprite.hpp"

namespace Manager {

    struct State {
        int source;
        std::string search;
        bool operator==(const State& other) const = default;
    };

    void SetSearch(std::string value);
    void SetSource(int source);
    void RequestMorePlaylists();

    State GetState();
    std::vector<Playlist*> GetPlaylists();

    void SelectPlaylist(int playlistIdx);
    Playlist* GetSelectedPlaylist();
    bool SelectedPlaylistOwned();

    void GetPlaylistCover(Playlist* playlist, std::function<void(UnityEngine::Sprite*)> callback);
    void GetPlaylistSongs(Playlist* playlist, std::function<void(std::vector<std::optional<BeatSaver::Beatmap>>)> callback);
    void GetSongCover(BeatSaver::Beatmap* song, std::function<void(UnityEngine::Sprite*)> callback);
    void GetPlaylistFile(Playlist* playlist, std::function<void(PlaylistCore::BPList)> callback);
};
