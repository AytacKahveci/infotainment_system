#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

#include "gtest/gtest.h"
#include <grpc++/grpc++.h>
#include "google/protobuf/empty.pb.h"

#include "media.pb.h"
#include "media.grpc.pb.h"


using namespace testing;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;

using media::MediaControlRequest;
using media::MediaControlResponse;
using media::MediaPlaybackStatus;
using media::MediaService;
using media::MediaPlaybackStatus_PlaybackState;
using google::protobuf::Empty;


class MediaServiceTestFixture : public Test
{
public:
  MediaServiceTestFixture()
  {
    mStub = MediaService::NewStub(
        grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials()));
  }

  void SetUp() override
  {
    std::cout << "Running SetUp for a test." << std::endl;
  }

  void TearDown() override
  {
    std::cout << "Running TearDown for a test." << std::endl;
  }

protected:
  std::unique_ptr<MediaService::Stub> mStub;
};


// GivenControl_WhenServiceIsRunning_ExpectPlaySong
TEST_F(MediaServiceTestFixture, GivenControl_WhenServiceIsRunning_ExpectPlaySong)
{
  std::cout << "Executing Test: GivenControl_WhenServiceIsRunning_ExpectPlaySong" << std::endl;

  MediaControlRequest request;
  request.set_play_track_id(1);
  
  MediaControlResponse response;
  ClientContext context;

  Status status = mStub->ControlMedia(&context, request, &response);

  EXPECT_TRUE(status.ok()) << "gRPC call failed: " << status.error_message();
  EXPECT_TRUE(response.success()) << "Media control command failed: " << response.message();
  EXPECT_EQ(response.message(), "Command executed successfully");
}

// GivenControl_WhenServiceIsRunning_ExpectPauseResume
TEST_F(MediaServiceTestFixture, GivenControl_WhenServiceIsRunning_ExpectPauseResume)
{
    std::cout << "Executing Test: GivenControl_WhenServiceIsRunning_ExpectPauseResume" << std::endl;

    MediaControlRequest playRequest;
    playRequest.set_play_track_id(1);
    MediaControlResponse playResponse;
    ClientContext playContext;
    Status playStatus = mStub->ControlMedia(&playContext, playRequest, &playResponse);
    ASSERT_TRUE(playStatus.ok()) << "Failed to start playing for pause test.";
    ASSERT_TRUE(playResponse.success()) << "Failed to start playing for pause test: " << playResponse.message();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait for service update

    // Send Pause command
    MediaControlRequest pauseRequest;
    pauseRequest.set_pause(true);
    MediaControlResponse pauseResponse;
    ClientContext pauseContext;
    Status pauseStatus = mStub->ControlMedia(&pauseContext, pauseRequest, &pauseResponse);

    EXPECT_TRUE(pauseStatus.ok()) << "Pause gRPC call failed: " << pauseStatus.error_message();
    EXPECT_TRUE(pauseResponse.success()) << "Pause command failed: " << pauseResponse.message();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait for service update

    // Send Resume command
    MediaControlRequest resumeRequest;
    resumeRequest.set_pause(false);
    MediaControlResponse resumeResponse;
    ClientContext resumeContext;
    Status resumeStatus = mStub->ControlMedia(&resumeContext, resumeRequest, &resumeResponse);

    EXPECT_TRUE(resumeStatus.ok()) << "Resume gRPC call failed: " << resumeStatus.error_message();
    EXPECT_TRUE(resumeResponse.success()) << "Resume command failed: " << resumeResponse.message();
}

// GivenStreamRequest_WhenServicePlays_ExpectStatusUpdates
TEST_F(MediaServiceTestFixture, GivenStreamRequest_WhenServicePlays_ExpectStatusUpdates)
{
    std::cout << "Executing Test: GivenStreamRequest_WhenServicePlays_ExpectStatusUpdates" << std::endl;

    ClientContext context;
    std::unique_ptr<ClientReader<MediaPlaybackStatus>> reader(mStub->StreamPlaybackStatus(&context, Empty()));

    MediaPlaybackStatus initialStatus;
    ASSERT_TRUE(reader->Read(&initialStatus)) << "Failed to read initial playback status.";
    std::cout << "Initial status: " << initialStatus.state() << std::endl;

    std::thread playThread([this]() {
        MediaControlRequest request;
        request.set_play_track_id(1);
        MediaControlResponse response;
        ClientContext controlContext;
        Status status = mStub->ControlMedia(&controlContext, request, &response);
        ASSERT_TRUE(status.ok()) << "Play command failed in separate thread.";
        ASSERT_TRUE(response.success()) << "Play command response error: " << response.message();
    });

    // Wait for update in the main thread
    MediaPlaybackStatus currentStatus;
    bool playingReceived = false;

    auto startTime = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - startTime < std::chrono::seconds(5)) { // Wait for 5 seconds
        if (reader->Read(&currentStatus)) {
            std::cout << "Received status update: "
                      << "State: " << currentStatus.state()
                      << ", Title: " << currentStatus.current_track_title()
                      << ", Elapsed: " << currentStatus.elapsed_time() << std::endl;

            if (currentStatus.state() == media::MediaPlaybackStatus_PlaybackState_PLAYING &&
                !currentStatus.current_track_title().empty()) {
                playingReceived = true;
                break; // Received expected state exit the loop
            }
        } else {
            std::cerr << "Stream read failed or stream ended prematurely." << std::endl;
            break;
        }
    }
    
    playThread.join();
    std::cout << "Thread is joined" << std::endl;

    context.TryCancel();
    EXPECT_TRUE(playingReceived) << "Did not receive PLAYING status update with track title.";
}


int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
