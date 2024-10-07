#include "manager.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "config.hpp"
#include "customtypes/mainmenu.hpp"
#include "customtypes/playlistdetail.hpp"
#include "customtypes/playlistlist.hpp"
#include "main.hpp"
#include "playlistcore/shared/Utils.hpp"
#include "web-utils/shared/WebUtils.hpp"

using namespace PlaylistDownloader;

// more than 50 will be invalid for one beatsaver api call
#define PLAYLIST_SONGS_PAGE_SIZE 10

// https://gist.github.com/jagt/5797948
template <class K, class V, int MaxSize = -1>
class cache {
   private:
    struct Entry {
        K key;
        V value;
        Entry* prev;
        Entry* next;
    };

    std::map<K, Entry*> dict;
    Entry *head, *tail;
    cache(cache const& rhs);
    cache& operator=(cache const& rhs);

    Entry* detach(Entry* entry) {
        entry->prev->next = entry->next;
        entry->next->prev = entry->prev;
        return entry;
    }

    void push_front(Entry* entry) {
        entry->next = head->next;
        entry->next->prev = entry;
        entry->prev = head;
        head->next = entry;
    }

   public:
    cache() : dict() {
        head = new Entry();
        tail = new Entry();
        head->next = tail;
        tail->prev = head;
    }

    ~cache() {
        while (head->next != tail) {
            delete detach(head->next);
        }
        delete head;
        delete tail;
    }

    void add(K const& key, V const& value) {
        Entry* entry = new Entry();
        entry->key = key;
        entry->value = value;
        push_front(entry);
        dict[key] = entry;
        if constexpr (MaxSize > 0) {
            if (size() > MaxSize)
                drop();
        }
    }

    V& get(K const& key) {
        Entry* entry = dict[key];
        // move to head
        detach(entry);
        push_front(entry);
        return entry->value;
    }

    bool has(K const& key) const { return dict.contains(key); }

    void drop() {
        Entry* entry = detach(tail->prev);
        dict.erase(entry->key);
        delete entry;
    }

    size_t size() const {
        size_t size = 0;
        Entry* p = head;
        while (p->next != tail) {
            ++size;
            p = p->next;
        }
        return size;
    }
};

template <class T>
struct dumb_function_copyable {
    dumb_function_copyable(T&& arg) : arg(std::move(arg)){};
    dumb_function_copyable(dumb_function_copyable<T> const& other) : arg(std::move(const_cast<T&>(other.arg))){};
    T& get() { return arg; }

   private:
    T arg;
};

struct CallbackSpriteRequest : WebUtils::GenericRequest<WebUtils::SpriteResponse> {
    CallbackSpriteRequest(std::string url, std::function<void(UnityEngine::Sprite*)> callback) : GenericRequest({url}), callback(callback) {
        this->url.noEscape = true;
    }
    std::function<void(UnityEngine::Sprite*)> callback;
    int retriesLeft = 2;
};

namespace Manager {
    int source = -1;
    int page = 0;
    int receivedPages = 0;
    bool done = false;
    std::string search = "";

    std::vector<std::unique_ptr<Playlist>> playlists = {};
    int selectedPlaylist = -1;

    int songsPage = 0;
    int receivedSongsPages = 0;
    bool songsDone = false;

    std::vector<std::optional<BeatSaver::Models::Beatmap>> songs = {};

    cache<std::string, UnityEngine::Sprite*, 50> cachedPlaylistCovers = {};
    cache<std::string, PlaylistCore::BPList, 100> cachedPlaylists = {};

    void Refresh() {
        logger.debug("Refreshing playlists");
        page = receivedPages = 0;
        done = false;
        playlists.clear();
        MainMenu::SetDetailShown(false);
        PlaylistList::GetInstance()->SetLoading(true);
        RequestMorePlaylists();
    }
    void RefreshSelected() {
        logger.debug("Refreshing playlist selection");
        songsPage = receivedSongsPages = 0;
        songsDone = false;
        songs.clear();
        MainMenu::SetDetailShown(GetSelectedPlaylist() != nullptr);
        PlaylistDetail::GetInstance()->SetLoading(true);
        PlaylistDetail::GetInstance()->Refresh(true);
        RequestMoreSongs();
    }
    void Invalidate() {
        playlists.clear();
        while (cachedPlaylistCovers.size() > 0)
            cachedPlaylistCovers.drop();
        while (cachedPlaylists.size() > 0)
            cachedPlaylists.drop();
    }

    void SetSearch(std::string value) {
        logger.debug("Searching \"{}\"", value);
        if (value == search)
            return;
        search = value;
        Refresh();
    }
    void SetSource(int newSource) {
        logger.debug("Source set to {}", newSource);
        if (newSource == source)
            return;
        source = newSource;
        Refresh();
    }
    void RequestMorePlaylists() {
        if (page > receivedPages || done)
            return;
        bool firstRequest = page == 0;
        logger.info("Requesting playlists from {} search \"{}\"", source, search);
        auto callback = [pastState = GetState(), firstRequest](std::vector<std::unique_ptr<Playlist>> newPlaylists) mutable {
            using capture_fix = dumb_function_copyable<std::vector<std::unique_ptr<Playlist>>>;
            auto arg = capture_fix(std::move(newPlaylists));

            BSML::MainThreadScheduler::Schedule([pastState, firstRequest, arg]() mutable {
                if (pastState != GetState())
                    return;
                if (!arg.get().empty()) {
                    playlists.insert(playlists.end(), std::make_move_iterator(arg.get().begin()), std::make_move_iterator(arg.get().end()));
                } else
                    done = true;
                receivedPages++;
                PlaylistList::GetInstance()->SetLoading(false);
                PlaylistList::GetInstance()->Refresh(firstRequest);
            });
        };
        switch (source) {
            case 0:
                GetBeatSaverPlaylists(callback, page, search);
                break;
            case 1:
                GetHitbloqPlaylists(callback, page, search);
                break;
            case 2:
                GetAccSaberPlaylists(callback, page, search);
                break;
            case 3:
                GetBeatLeaderPlaylists(callback, page, search);
                break;
        }
        page++;
    }

    State GetState() {
        // no change to state if filters are changed on a source that doesn't use them
        bool curated = SourceHasFilters(source) ? getConfig().curated.GetValue() : false;
        bool includeEmpty = SourceHasFilters(source) ? getConfig().includeEmpty.GetValue() : false;
        std::string sort = SourceHasFilters(source) ? getConfig().sort.GetValue() : "";
        return State{
            .source = source,
            .search = search,
            .curated = curated,
            .includeEmpty = includeEmpty,
            .sort = sort,
        };
    }
    std::vector<Playlist*> GetPlaylists() {
        std::vector<Playlist*> ret = {};
        ret.reserve(playlists.size());

        for (auto& playlist : playlists)
            ret.push_back(playlist.get());

        return ret;
    }

    void SelectPlaylist(int playlistIdx) {
        logger.debug("Playlist set to {}", playlistIdx);
        selectedPlaylist = playlistIdx;
        RefreshSelected();
    }
    Playlist* GetSelectedPlaylist() {
        if (selectedPlaylist < 0 || selectedPlaylist > playlists.size())
            return nullptr;
        return playlists[selectedPlaylist].get();
    }

    void RequestMoreSongs() {
        auto playlist = GetSelectedPlaylist();
        if (!playlist || songsPage > receivedSongsPages || songsDone)
            return;
        logger.debug("Getting playlist songs {} page {}", playlist->Title, songsPage);
        auto callback = [currentPage = songsPage, playlist](std::optional<PlaylistCore::BPList> downloaded) {
            if (playlist != GetSelectedPlaylist() || !downloaded)
                return;
            auto& bpSongs = downloaded->Songs;
            int start = PLAYLIST_SONGS_PAGE_SIZE * currentPage;
            int end = PLAYLIST_SONGS_PAGE_SIZE * (currentPage + 1);
            end = std::min(end, (int) bpSongs.size());
            int count = end - start;
            if (count <= 0) {
                receivedSongsPages++;
                songsDone = true;
                PlaylistDetail::GetInstance()->SetLoading(false);
                PlaylistDetail::GetInstance()->Refresh(false);
                return;
            }
            std::vector<std::string> hashes(count);
            for (int i = start; i < end; i++)
                hashes.emplace_back(PlaylistCore::Utils::GetLevelHash(bpSongs[i].LevelID));
            auto onGet = [hashes, playlist](BeatSaver::API::BeatmapMapResponse response) {
                if (playlist != GetSelectedPlaylist())
                    return;
                if (!response.responseData) {
                    logger.error("Failed to get beatmaps {} {}", response.httpCode, response.curlStatus);
                    BSML::MainThreadScheduler::Schedule([playlist]() {
                        if (playlist != GetSelectedPlaylist())
                            return;
                        // don't increment page if it fails
                        PlaylistDetail::GetInstance()->SetLoading(false);
                        PlaylistDetail::GetInstance()->Refresh(false);
                    });
                    return;
                }
                std::vector<std::optional<BeatSaver::Models::Beatmap>> maps;
                for (auto& hash : hashes) {
                    bool found = false;
                    // iterate to not have to worry about case
                    for (auto& [_, map] : *response.responseData) {
                        for (auto& version : map.Versions) {
                            if (PlaylistCore::Utils::CaseInsensitiveEquals(version.Hash, hash)) {
                                found = true;
                                maps.emplace_back(map);
                                break;
                            }
                        }
                        if (found)
                            break;
                    }
                    if (!found)
                        maps.emplace_back(std::nullopt);
                }
                BSML::MainThreadScheduler::Schedule([playlist, maps]() {
                    if (playlist != GetSelectedPlaylist())
                        return;
                    for (auto& map : maps)
                        songs.push_back(map);
                    receivedSongsPages++;
                    PlaylistDetail::GetInstance()->SetLoading(false);
                    PlaylistDetail::GetInstance()->Refresh(false);
                });
            };
            BEATSAVER_PLUSPLUS_GET_ASYNC(BeatSaver::API::GetBeatmapsByHashesURLOptions, onGet, hashes);
        };
        GetPlaylistFile(playlist, callback);
        songsPage++;
    }
    std::vector<BeatSaver::Models::Beatmap*> GetSongs() {
        std::vector<BeatSaver::Models::Beatmap*> ret = {};
        ret.reserve(songs.size());

        for (auto& song : songs) {
            if (song.has_value())
                ret.push_back(&*song);
        }
        return ret;
    }

    void GetSprite(std::string url, std::function<void(UnityEngine::Sprite*)> callback) {
        using Retry = WebUtils::RatelimitedDispatcher::RetryOptions;
        static WebUtils::RatelimitedDispatcher spriteGetter;
        static bool initialized = false;
        static std::map<std::string, CallbackSpriteRequest*> awaitingSprites;
        if (!initialized) {
            initialized = true;
            spriteGetter.maxConcurrentRequests = 4;
            spriteGetter.rateLimitTime = std::chrono::milliseconds(100);
            spriteGetter.onRequestFinished = [](bool success, WebUtils::IRequest* request) -> std::optional<Retry> {
                auto cast = dynamic_cast<CallbackSpriteRequest*>(request);
                if (!cast)
                    return std::nullopt;

                auto response = cast->targetResponse;
                auto sprite = response.responseData.value_or(nullptr).unsafePtr();
                if (!sprite) {
                    logger.error("failed to download sprite from {}: {} {}", cast->url.fullURl(), response.httpCode, response.curlStatus);
                    if (0 < cast->retriesLeft--)
                        return Retry(std::chrono::milliseconds(100));
                }
                // technically, the url should be removed from awaitingSprites here with a lock
                BSML::MainThreadScheduler::Schedule([callback = std::move(cast->callback), url = cast->url.url, sprite]() {
                    if (callback)
                        callback(sprite);
                    // no lock needed if on main thread
                    awaitingSprites.erase(url);
                });

                return std::nullopt;
            };
        }
        if (!awaitingSprites.contains(url)) {
            auto request = std::make_unique<CallbackSpriteRequest>(url, callback);
            awaitingSprites[url] = request.get();
            spriteGetter.AddRequest(std::move(request));
            spriteGetter.StartDispatchIfNeeded();
        } else
            awaitingSprites[url]->callback = callback;
    }

    void GetPlaylistCover(Playlist* playlist, std::function<void(UnityEngine::Sprite*)> callback) {
        logger.debug("Getting playlist cover {}", playlist->Title);
        auto url = playlist->ImageURL;
        if (cachedPlaylistCovers.has(url)) {
            callback(cachedPlaylistCovers.get(url));
            return;
        }
        callback = [url, fallback = playlist->FallbackImage, callback = std::move(callback)](UnityEngine::Sprite* sprite) {
            if (sprite)
                cachedPlaylistCovers.add(url, sprite);
            callback(sprite ? sprite : BSML::Utilities::LoadSpriteRaw(ArrayW<uint8_t>(*fallback)));
        };
        GetSprite(url, callback);
    }
    void GetSongCover(BeatSaver::Models::Beatmap* song, std::function<void(UnityEngine::Sprite*)> callback) {
        logger.debug("Getting song cover {}", song->GetName());
        std::string url = song->Versions.front().CoverURL;
        GetSprite(url, callback);
    }

    PlaylistCore::Playlist* PlaylistOwned(Playlist const* downloaderPlaylist) {
        auto allLists = PlaylistCore::GetLoadedPlaylists();
        for (auto& loaded : allLists) {
            auto& cdata = loaded->playlistJSON.CustomData;
            if (cdata.has_value() && cdata->SyncURL == downloaderPlaylist->PlaylistURL)
                return loaded;
        }
        return nullptr;
    }
    PlaylistCore::Playlist* SelectedPlaylistOwned() {
        auto playlist = GetSelectedPlaylist();
        if (!playlist)
            return nullptr;
        return PlaylistOwned(playlist);
    }
    void GetPlaylistFile(Playlist* playlist, std::function<void(std::optional<PlaylistCore::BPList>)> callback) {
        logger.debug("Getting playlist file {}", playlist->Title);
        auto url = playlist->PlaylistURL;
        if (cachedPlaylists.has(url)) {
            callback(cachedPlaylists.get(url));
            return;
        }
        WebUtils::GetAsync<WebUtils::StringResponse>({url}, [url, callback](WebUtils::StringResponse response) {
            BSML::MainThreadScheduler::Schedule([response, url, callback]() {
                if (!response.responseData) {
                    logger.error("Get playlist file failed {} {}", response.httpCode, response.curlStatus);
                    callback(std::nullopt);
                    return;
                }
                PlaylistCore::BPList list;
                try {
                    ReadFromString(*response.responseData, list);
                    if (!list.CustomData.has_value())
                        list.CustomData.emplace();
                    auto& syncUrl = list.CustomData->SyncURL;
                    if (!syncUrl.has_value() || syncUrl->empty())
                        syncUrl = url;
                } catch (JSONException const& exc) {
                    logger.error("Failed to deserialize playlist {}: {}", *response.responseData, exc.what());
                    callback(std::nullopt);
                    return;
                }
                cachedPlaylists.add(url, list);
                callback(list);
            });
        });
    }
};
