#ifndef QT_INFORMATION_UI_SRC_MEDIA_CLIENT_H_
#define QT_INFORMATION_UI_SRC_MEDIA_CLIENT_H_

#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <grpc++/grpc++.h>
#include "media.grpc.pb.h"
#include "media.pb.h"
#include "google/protobuf/empty.pb.h"
#include "qt_information_ui/include/media_client_interface.h"

namespace qt_information_ui
{
  class MediaClient : public MediaClientInterface
  {
  public:
    MediaClient(std::shared_ptr<grpc::Channel> channel);
    virtual ~MediaClient();

    bool PlaySong(const Song& song) override;
    std::vector<Song> GetSongs() override;
    const PlaybackStatus& GetPlaybackStatus() override;

  private:
    std::unique_ptr<media::MediaService::Stub> mStub;
    PlaybackStatus mPlaybackStatus{};
    bool mIsStopRequested{false};
    std::thread mReceiverThread{};
    std::mutex mReceiverMutex{};

    grpc::Status PlaySongRequest(const Song& song,
                                 media::MediaControlResponse* response);
    
    grpc::Status GetPlaylistRequest(media::GetPlaylistResponse* response);

    void ReceiverThread();
    
  };
} // namespace qt_information_ui
#endif // QT_INFORMATION_UI_SRC_MEDIA_CLIENT_H_
