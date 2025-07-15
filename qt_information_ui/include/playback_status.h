#ifndef QT_INFORMATION_UI_INCLUDE_PLAYBACK_STATUS_H_
#define QT_INFORMATION_UI_INCLUDE_PLAYBACK_STATUS_H_

#include <string>
#include <memory>
#include "qt_information_ui/include/song.h"

namespace qt_information_ui
{
  /**
   * @brief Represents Playback state
   */
  enum class PlaybackState : uint8_t
  {
    STOPPED = 0,
    PLAYING = 1,
    PAUSED = 2,
  };

  /**
   * @brief Represents Playback status of the media service
   */
  class PlaybackStatus
  {
  public:
    explicit PlaybackStatus() = default;
    explicit PlaybackStatus(std::shared_ptr<Song> pSong) : mpSong(pSong) {}

    PlaybackState mState{PlaybackState::STOPPED};
    std::shared_ptr<Song> mpSong{nullptr};
    std::uint32_t mElapsedTime{0};
    std::uint8_t mVolumeLevel{0};
    std::int64_t mCurrentSongIndex{-1};
  };
} // namespace qt_information_ui
#endif // QT_INFORMATION_UI_INCLUDE_PLAYBACK_STATUS_H_
