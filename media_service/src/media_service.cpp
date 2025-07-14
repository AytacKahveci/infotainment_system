#include "media_service.h"

namespace media_service
{
  MediaServiceImpl::MediaServiceImpl(const std::vector<Song> &songs) : mSongs(songs)
  {
    mCurrentStatus.mState = PlaybackState::STOPPED;

    mPlaybackUpdaterThread = std::thread(&MediaServiceImpl::UpdatePlaybackStatus, this);
  }

  MediaServiceImpl::~MediaServiceImpl()
  {
    mStopPlaybackUpdater = true;
    mCV.notify_all();
    mPlaybackUpdaterThread.join();

    std::lock_guard<std::mutex> lock(mWriterMutex);
    for (auto &writer : mActiveWriters)
    {
      // writer->WriteLast(Status::OK);
    }
  }

  void MediaServiceImpl::Play() {}

  void MediaServiceImpl::Pause() {}

  const std::vector<Song> &MediaServiceImpl::GetSongList() const 
  {
    return mSongs;
  }

  const PlaybackStatus &MediaServiceImpl::GetPlaybackStatus() const 
  {
    return mCurrentStatus;
  }

  Status MediaServiceImpl::ControlMedia(ServerContext *context,
                        const MediaControlRequest *request,
                        MediaControlResponse *response)
  {
    std::cout << "ControlMedia is called" << std::endl;
    std::lock_guard<std::mutex> lock(mPlaybackMutex);
    response->set_success(true);
    response->set_message("Command executed successfully");

    if (request->has_play_track_id())
    {
      uint64_t trackId = request->play_track_id();
      auto it = std::find_if(mSongs.begin(), mSongs.end(), [trackId] (const Song& song) {
        return song.GetId() == trackId;
      });
      if (it != mSongs.end())
      {
        mCurrentStatus.mpSong = std::make_shared<Song>(*it);
        mCurrentStatus.mCurrentSongIndex = std::distance(mSongs.begin(), it);
        mCurrentStatus.mState = PlaybackState::PLAYING;
        mCurrentStatus.mElapsedTime = 0;
        std::cout << "Playing: " << it->GetTrackTitle() << std::endl;
      }
      else
      {
        response->set_success(false);
        response->set_message("Song couldn't found");
      }
    }
    else if (request->has_pause())
    {
      if (request->pause())
      {
        mCurrentStatus.mState = PlaybackState::PAUSED;
        std::cout << "Playback paused" << std::endl;
      }
      else
      {
        mCurrentStatus.mState = PlaybackState::PLAYING;
        std::cout << "Playback resumed" << std::endl;
      }
    }
    else if (request->has_stop())
    {
      mCurrentStatus.mpSong = nullptr;
      mCurrentStatus.mState = PlaybackState::STOPPED;
      mCurrentStatus.mElapsedTime = 0;
      std::cout << "Playback stopped" << std::endl;
    }
    else if (request->has_set_volume())
    {
      uint32_t newVolume = request->set_volume();
      if (newVolume <= 100)
      {
        mCurrentStatus.mVolumeLevel = newVolume;
        std::cout << "Volume is set to: " << newVolume << std::endl;
      }
      else
      {
        response->set_success(false);
        response->set_message("Invalid volume level. Must be 0-100");
      }
    }
    else if (request->has_skip_next())
    {
      if (!mCurrentStatus.mpSong)
      {
        response->set_success(false);
        response->set_message("No track currently playing to skip.");
      }

      mCurrentStatus.mCurrentSongIndex++;
      if (mCurrentStatus.mCurrentSongIndex >= mSongs.size())
      {
        mCurrentStatus.mCurrentSongIndex = 0;
      }

      mCurrentStatus.mpSong = std::make_shared<Song>(mSongs[mCurrentStatus.mCurrentSongIndex]); 
    }
    else if (request->has_skip_prev())
    {
      if (!mCurrentStatus.mpSong)
      {
        response->set_success(false);
        response->set_message("No track currently playing to skip.");
      }

      mCurrentStatus.mCurrentSongIndex--;
      if (mCurrentStatus.mCurrentSongIndex < 0)
      {
        mCurrentStatus.mCurrentSongIndex = mSongs.size() - 1;
      }

      mCurrentStatus.mpSong = std::make_shared<Song>(mSongs[mCurrentStatus.mCurrentSongIndex]);
    }
    else
    {
      response->set_success(false);
      response->set_message("Unknown command");
    }

    mCV.notify_all();
    return Status::OK;
  }

  Status MediaServiceImpl::StreamPlaybackStatus(ServerContext *context,
                                                const Empty *request,
                                                ServerWriter<MediaPlaybackStatus> *writer)
  {
    {
      std::lock_guard<std::mutex> lock(mWriterMutex);
      mActiveWriters.insert(writer);
      std::cout << "New client connected to StreamPlaybackStatus. Total: " << mActiveWriters.size() << std::endl;
    }

    while (!context->IsCancelled())
    {
      std::unique_lock<std::mutex> lock(mPlaybackMutex);
      
      mCV.wait(lock, [this, &context] {
        return context->IsCancelled() || mStatusChanged || mStopPlaybackUpdater;
      });

      if (context->IsCancelled() || mStopPlaybackUpdater)
      {
        break;
      }

      mStatusChanged = false;

      MediaPlaybackStatus status;
      status.set_state(MediaPlaybackStatus_PlaybackState(mCurrentStatus.mState));
      status.set_volume_level(mCurrentStatus.mVolumeLevel);

      if (mCurrentStatus.mpSong)
      {
        status.set_current_track_title(mCurrentStatus.mpSong->GetTrackTitle());
        status.set_current_artist_name(mCurrentStatus.mpSong->GetArtistName());
        status.set_track_duration(mCurrentStatus.mpSong->GetDuration());
        status.set_elapsed_time(mCurrentStatus.mElapsedTime);
      }
      else
      {
        status.set_current_track_title("");
        status.set_current_artist_name("");
        status.set_track_duration(0);
        status.set_elapsed_time(0);
      }

      if (!writer->Write(status))
      {
        std::cerr << "Failed to write to client, connection likely lost" << std::endl;
        break;
      }
      std::cout << "Client is updated" << std::endl;
    }

    {
      std::lock_guard<std::mutex> lock(mWriterMutex);
      mActiveWriters.erase(writer);
      std::cout << "Client disconnected from StreamPlaybackStatus. Total: " << mActiveWriters.size() << std::endl;
    } 
    return Status::OK;
  }

  void MediaServiceImpl::UpdatePlaybackStatus()
  {
    while (!mStopPlaybackUpdater)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      std::lock_guard<std::mutex> lock(mPlaybackMutex);
      if (mCurrentStatus.mState == PlaybackState::PLAYING && mCurrentStatus.mpSong != nullptr)
      {
        mCurrentStatus.mElapsedTime += 500;
        if (mCurrentStatus.mElapsedTime >= mCurrentStatus.mpSong->GetDuration())
        {
          // Get next song
          mCurrentStatus.mCurrentSongIndex++;
          mCurrentStatus.mElapsedTime = 0;
          if (mCurrentStatus.mCurrentSongIndex > mSongs.size())
          {
            mCurrentStatus.mCurrentSongIndex = -1;
            mCurrentStatus.mpSong = nullptr;
            mCurrentStatus.mState = PlaybackState::STOPPED;
            std::cout << "Playlist ended" << std::endl;
          }
          else
          {
            mCurrentStatus.mpSong = std::make_shared<Song>(mSongs[mCurrentStatus.mCurrentSongIndex]);
            std::cout << "Moving to next song" << std::endl;
          }
        }
      }

      mStatusChanged = true;
      mCV.notify_all();
    }
    std::cout << "Playback updater thread is stopped" << std::endl;
  }

} // namespace media_service
