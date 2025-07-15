#ifndef QT_INFORMATION_UI_INCLUDE_SONG_H_
#define QT_INFORMATION_UI_INCLUDE_SONG_H_

#include <string>

namespace qt_information_ui
{

  /**
   * @brief Represents a song information
   *
   */
  class Song
  {
  public:
    explicit Song(std::uint64_t id, const std::string &trackTitle,
                  const std::string &artistName, std::uint32_t duration)
        : mId{id}, mTrackTitle{trackTitle}, mArtistName{artistName}, mDuration{duration}
    {
    }

    std::uint64_t GetId() const
    {
      return mId;
    }

    const std::string &GetTrackTitle() const
    {
      return mTrackTitle;
    }

    const std::string &GetArtistName() const
    {
      return mArtistName;
    }

    std::uint32_t GetDuration() const
    {
      return mDuration;
    }

  private:
    std::uint64_t mId;
    std::string mTrackTitle;
    std::string mArtistName;
    std::uint32_t mDuration;
  };

} // namespace qt_information_ui
#endif // QT_INFORMATION_UI_INCLUDE_SONG_H_
