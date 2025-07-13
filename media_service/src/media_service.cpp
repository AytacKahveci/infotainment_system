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
    if (mPlaybackUpdaterThread.joinable())
    {
      mPlaybackUpdaterThread.join();
    }

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
    }

    {
      std::lock_guard<std::mutex> lock(mWriterMutex);
      mActiveWriters.erase(writer);
      std::cout << "Client disconnected from StreamPlaybackStatus. Total: " << mActiveWriters.size() << std::endl;
    } 
    return Status::OK;
  }

  void MediaServiceImpl::UpdatePlaybackStatus()
  {}

} // namespace media_service
