#ifndef MEDIA_SERVICE_INCLUDE_SONG_H_
#define MEDIA_SERVICE_INCLUDE_SONG_H_

#include <string>

namespace media_service
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

} // namespace media_service
#endif // MEDIA_SERVICE_INCLUDE_SONG_H_
