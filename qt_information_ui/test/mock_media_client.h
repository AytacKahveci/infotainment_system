#ifndef QT_INFORMATION_UI_TEST_MOCK_MEDIA_CLIENT_H_
#define QT_INFORMATION_UI_TEST_MOCK_MEDIA_CLIENT_H_

#include "gmock/gmock.h"
#include "qt_information_ui/include/media_client_interface.h"
#include "qt_information_ui/include/song.h"
#include "qt_information_ui/include/playback_status.h"

namespace qt_information_ui
{

  class MockMediaClient : public MediaClientInterface
  {
  public:
    MOCK_METHOD(std::vector<Song>, GetSongs, (), (override));
    MOCK_METHOD(bool, PlaySong, (const Song &song), (override));
    MOCK_METHOD(bool, PauseSong, (bool pause), (override));
    MOCK_METHOD(bool, StopSong, (), (override));
    MOCK_METHOD(bool, PlayNext, (), (override));
    MOCK_METHOD(bool, PlayPrevious, (), (override));
    MOCK_METHOD(bool, SetVolume, (uint32_t volume), (override));
    MOCK_METHOD(const PlaybackStatus &, GetPlaybackStatus, (), (override));

    MOCK_METHOD(void, SetHandler, (const HandlerT &handler), (override));

    void CallPlaybackStatusHandler(const PlaybackStatus &status)
    {
      if (mHandler)
      {
        mHandler(status);
      }
    }
  };

} // namespace qt_information_ui

#endif // QT_INFORMATION_UI_TEST_MOCK_MEDIA_CLIENT_H_