#ifndef QT_INFORMATION_UI_INCLUDE_MEDIA_CLIENT_INTERFACE_H_
#define QT_INFORMATION_UI_INCLUDE_MEDIA_CLIENT_INTERFACE_H_

#include <memory>
#include <vector>

#include "qt_information_ui/include/song.h"
#include "qt_information_ui/include/playback_status.h"

namespace qt_information_ui
{
  class MediaClientInterface
  {
  public:
    virtual ~MediaClientInterface() = default;
    virtual std::vector<Song> GetSongs() = 0;
    virtual bool PlaySong(const Song& song) = 0;
    virtual const PlaybackStatus& GetPlaybackStatus() = 0;
  };
} // namespace qt_information_ui
#endif // QT_INFORMATION_UI_INCLUDE_MEDIA_CLIENT_INTERFACE_H_
