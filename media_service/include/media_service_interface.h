#ifndef MEDIA_SERVICE_INCLUDE_MEDIA_SERVICE_INTERFACE_H_
#define MEDIA_SERVICE_INCLUDE_MEDIA_SERVICE_INTERFACE_H_

#include <vector>
#include "media_service/include/song.h"
#include "media_service/include/playback_status.h"

namespace media_service
{
  /**
   * @brief Interface for MediaService
   */
  class MediaServiceInterface
  {
  public:
    virtual ~MediaServiceInterface() = default;
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual const std::vector<Song>& GetSongList() const = 0;
    virtual const PlaybackStatus& GetPlaybackStatus() const = 0;
  };
}
#endif // MEDIA_SERVICE_INCLUDE_MEDIA_SERVICE_INTERFACE_H_
