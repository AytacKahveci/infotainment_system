#include <chrono>
#include "qt_information_ui/src/media_client.h"

namespace qt_information_ui
{
  MediaClient::MediaClient(std::shared_ptr<grpc::Channel> channel)
      : mStub(media::MediaService::NewStub(channel))
  {
  }

  MediaClient::~MediaClient()
  {
    mIsStopRequested = true;
    mReceiverThread.join();
  }

  bool MediaClient::PlaySong(const Song& song)
  {
    media::MediaControlResponse response;
    if (!PlaySongRequest(song, &response).ok())
    {
      std::cout << "PlaySong request failed" << std::endl;
      return false;
    }

    return true;
  }

  std::vector<Song> MediaClient::GetSongs()
  {
    std::vector<Song> songs;
    media::GetPlaylistResponse response;
    if (!GetPlaylistRequest(&response).ok())
    {
      std::cout << "GetPlayList request failed" << std::endl;
      return songs;
    }

    const auto& tracks = response.tracks();
    for (const auto& track : tracks)
    {
      songs.emplace_back(track.id(), track.title(), track.artist(), track.duration());
    }
    return songs;
  }

  const PlaybackStatus& MediaClient::GetPlaybackStatus() 
  {
    std::lock_guard<std::mutex> lock(mReceiverMutex);
    return mPlaybackStatus;
  }

  grpc::Status MediaClient::PlaySongRequest(const Song& song,
                                            media::MediaControlResponse* response)
  {
    grpc::ClientContext context;
    media::MediaControlRequest request;
    request.set_play_track_id(song.GetId());
    return mStub->ControlMedia(&context, request, response);
  }

  grpc::Status MediaClient::GetPlaylistRequest(media::GetPlaylistResponse* response)
  {
    grpc::ClientContext context;
    media::GetPlayListRequest request;
    return mStub->GetPlaylist(&context, request, response);
  }


  void MediaClient::ReceiverThread()
  {
    grpc::ClientContext context;
    std::unique_ptr<grpc::ClientReader<media::MediaPlaybackStatus>> reader{nullptr};
    std::chrono::time_point start = std::chrono::steady_clock::now();

    while (!mIsStopRequested)
    {
      if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5))
      {
        std::cout << "Timeout exceeded in receiver thread" << std::endl;
      }

      if (!reader)
      {
        reader = mStub->StreamPlaybackStatus(&context, google::protobuf::Empty());
      }
      else
      {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }

      media::MediaPlaybackStatus currentStatus;
      if (reader->Read(&currentStatus))
      {
        std::lock_guard<std::mutex> lock(mReceiverMutex);
        start = std::chrono::steady_clock::now();

        std::string currentTrackTitle = currentStatus.current_track_title();
        std::string currentArtistName = currentStatus.current_artist_name();
        uint32_t trackDuration = currentStatus.track_duration();
        uint32_t elapsedTime = currentStatus.elapsed_time();
        uint32_t volumeLevel = currentStatus.volume_level();
      }
      else
      {
        std::cout << "Couldn't read from server" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
  }
} // namespace qt_information_ui
