#ifndef MEDIA_SERVICE_SRC_MEDIA_SERVICE_H_
#define MEDIA_SERVICE_SRC_MEDIA_SERVICE_H_

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <grpc++/grpc++.h>
#include "google/protobuf/empty.pb.h"
#include "media.grpc.pb.h"
#include "media_service/include/media_service_interface.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

using media::GetPlayListRequest;
using media::GetPlayListResponse;
using media::MediaControlRequest;
using media::MediaControlResponse;
using media::MediaPlaybackStatus;
using media::MediaService;
using media::MediaPlaybackStatus_PlaybackState;
using google::protobuf::Empty;

namespace media_service
{
  class MediaServiceImpl : public MediaService::Service, MediaServiceInterface
  {
  public:
    explicit MediaServiceImpl(const std::vector<Song> &songs);
    ~MediaServiceImpl() override;

    void Play() override;
    void Pause() override;
    const std::vector<Song> &GetSongList() const override;
    const PlaybackStatus &GetPlaybackStatus() const override;

  private:
    std::vector<Song> mSongs;
    PlaybackStatus mCurrentStatus;
    std::thread mPlaybackUpdaterThread;
    std::mutex mPlaybackMutex;
    std::condition_variable mCV;
    bool mStatusChanged{false};
    bool mStopPlaybackUpdater{false};

    std::set<ServerWriter<MediaPlaybackStatus>*> mActiveWriters;
    std::mutex mWriterMutex;

    Status ControlMedia(ServerContext *context,
                        const MediaControlRequest *request,
                        MediaControlResponse *response) override;

    Status StreamPlaybackStatus(ServerContext *context,
                                const Empty *request,
                                ServerWriter<MediaPlaybackStatus> *writer) override;

    Status GetPlayList(ServerContext *context,
                       const GetPlayListRequest *request,
                       GetPlayListResponse *response) override;

    void UpdatePlaybackStatus();
  };

}
#endif // MEDIA_SERVICE_SRC_MEDIA_SERVICE_H_
