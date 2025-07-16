#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <QApplication>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QStackedWidget>
#include <QTest>

#include "qt_information_ui/src/mainwindow.h"
#include "qt_information_ui/test/mock_media_client.h"
#include "qt_information_ui/include/song.h"
#include "qt_information_ui/include/playback_status.h"

using namespace qt_information_ui;
using namespace testing;

QApplication *g_app = nullptr;

class QApplicationInitializer : public testing::Environment
{
public:
  void SetUp() override
  {
    if (!g_app)
    {
      int argc = 0;
      char *argv[] = {nullptr};
      g_app = new QApplication(argc, argv);
      qDebug() << "QApplication initialized for tests.";
    }
  }

  void TearDown() override
  {
    if (g_app)
    {
      delete g_app;
      g_app = nullptr;
      qDebug() << "QApplication cleaned up after tests.";
    }
  }
};

testing::Environment *const env = testing::AddGlobalTestEnvironment(new QApplicationInitializer);

class MainWindowTest : public Test
{
protected:
  std::shared_ptr<MockMediaClient> mockMediaClient;
  std::unique_ptr<MainWindow> mainWindow;
  std::vector<Song> mockSongs;

  void SetUp() override
  {
    mockMediaClient = std::make_shared<MockMediaClient>();

    mockSongs.emplace_back(1, "Mock Song A", "Mock Artist 1", 180000);
    mockSongs.emplace_back(2, "Mock Song B", "Mock Artist 2", 240000);
    mockSongs.emplace_back(3, "Mock Song C", "Mock Artist 3", 210000);

    EXPECT_CALL(*mockMediaClient, GetSongs())
        .WillOnce(Return(mockSongs));

    EXPECT_CALL(*mockMediaClient, SetHandler(_))
        .WillOnce(SaveArg<0>(&mockMediaClient->mHandler));

    mainWindow = std::make_unique<MainWindow>(mockMediaClient);

  }

  void TearDown() override
  {
    mainWindow.reset();
  }

  // Helper to simulate a button click
  void clickButton(QPushButton *button)
  {
    ASSERT_TRUE(button != nullptr) << "Button is null!";
    QTest::mouseClick(button, Qt::LeftButton);
  }
};

TEST_F(MainWindowTest, InitialPageIsHomePage)
{
  ASSERT_EQ(mainWindow->findChild<QStackedWidget *>("stackedWidget")->currentWidget()->objectName().toStdString(), "homePage");
}

TEST_F(MainWindowTest, MediaButtonClickSwitchesToMediaPage)
{
  QPushButton *mediaButton = mainWindow->findChild<QPushButton *>("btn_media");
  ASSERT_NE(mediaButton, nullptr);

  clickButton(mediaButton);

  ASSERT_EQ(mainWindow->findChild<QStackedWidget *>("stackedWidget")->currentWidget()->objectName().toStdString(), "mediaPage");
}


TEST_F(MainWindowTest, PlaylistLoadsCorrectly)
{
  QListWidget *playlist = mainWindow->findChild<QListWidget *>("list_playlist");
  ASSERT_NE(playlist, nullptr);

  ASSERT_EQ(playlist->count(), mockSongs.size());
  for (int i = 0; i < mockSongs.size(); ++i)
  {
    EXPECT_EQ(playlist->item(i)->text().toStdString(), mockSongs[i].GetTrackTitle());
  }
}

TEST_F(MainWindowTest, PlayPauseButtonPlaysSongWhenStopped)
{
  clickButton(mainWindow->findChild<QPushButton *>("btn_media"));

  QListWidget *playlist = mainWindow->findChild<QListWidget *>("list_playlist");
  ASSERT_NE(playlist, nullptr);
  playlist->setCurrentRow(0);                                          
  clickButton(mainWindow->findChild<QPushButton *>("btn_play_pause"));

  PlaybackStatus initialStatus;
  initialStatus.mState = PlaybackState::STOPPED;
  mockMediaClient->CallPlaybackStatusHandler(initialStatus);

  QPushButton *playPauseButton = mainWindow->findChild<QPushButton *>("btn_play_pause");
  ASSERT_NE(playPauseButton, nullptr);

  clickButton(playPauseButton);

  PlaybackStatus playingStatus;
  playingStatus.mState = PlaybackState::PLAYING;
  playingStatus.mpSong = std::make_shared<Song>(mockSongs[0]);
  playingStatus.mElapsedTime = 0;
  playingStatus.mVolumeLevel = 50;
  mockMediaClient->CallPlaybackStatusHandler(playingStatus);

  EXPECT_EQ(playPauseButton->text().toStdString(), "Pause");
}

TEST_F(MainWindowTest, PlayPauseButtonPausesSongWhenPlaying)
{
  clickButton(mainWindow->findChild<QPushButton *>("btn_media"));

  QPushButton *playPauseButton = mainWindow->findChild<QPushButton *>("btn_play_pause");
  ASSERT_NE(playPauseButton, nullptr);

  PlaybackStatus playingStatus;
  playingStatus.mState = PlaybackState::PLAYING;
  playingStatus.mpSong = std::make_shared<Song>(mockSongs[0]);
  playingStatus.mElapsedTime = 10000;
  playingStatus.mVolumeLevel = 50;
  mockMediaClient->CallPlaybackStatusHandler(playingStatus);

  EXPECT_CALL(*mockMediaClient, PauseSong(true))
      .WillOnce(Return(true));

  clickButton(playPauseButton);

  PlaybackStatus pausedStatus;
  pausedStatus.mState = PlaybackState::PAUSED;
  pausedStatus.mpSong = std::make_shared<Song>(mockSongs[0]);
  pausedStatus.mElapsedTime = 10000;
  pausedStatus.mVolumeLevel = 50;
  mockMediaClient->CallPlaybackStatusHandler(pausedStatus);

  EXPECT_EQ(playPauseButton->text().toStdString(), "Play");
}

TEST_F(MainWindowTest, NextButtonCallsPlayNext)
{
  clickButton(mainWindow->findChild<QPushButton *>("btn_media"));

  EXPECT_CALL(*mockMediaClient, PlayNext())
      .WillOnce(Return(true));

  QPushButton *nextButton = mainWindow->findChild<QPushButton *>("btn_next");
  ASSERT_NE(nextButton, nullptr);

  clickButton(nextButton);
}

TEST_F(MainWindowTest, PreviousButtonCallsPlayPrevious)
{
  clickButton(mainWindow->findChild<QPushButton *>("btn_media"));

  EXPECT_CALL(*mockMediaClient, PlayPrevious())
      .WillOnce(Return(true));

  QPushButton *prevButton = mainWindow->findChild<QPushButton *>("btn_previous");
  ASSERT_NE(prevButton, nullptr);

  clickButton(prevButton);
}

TEST_F(MainWindowTest, VolumeSliderCallsSetVolume)
{
  clickButton(mainWindow->findChild<QPushButton *>("btn_media"));

  int expectedVolume = 75;

  EXPECT_CALL(*mockMediaClient, SetVolume(expectedVolume))
      .WillOnce(Return(true));

  QSlider *volumeSlider = mainWindow->findChild<QSlider *>("slider_volume");
  ASSERT_NE(volumeSlider, nullptr);

  volumeSlider->setValue(expectedVolume);
}

TEST_F(MainWindowTest, UIUpdatesOnPlaybackStatusChange)
{
  clickButton(mainWindow->findChild<QPushButton *>("btn_media"));

  PlaybackStatus testStatus;
  testStatus.mState = PlaybackState::PLAYING;
  testStatus.mpSong = std::make_shared<Song>(1, "Test Song Title", "Test Artist", 240000);
  testStatus.mElapsedTime = 65000;
  testStatus.mVolumeLevel = 60;

  mockMediaClient->CallPlaybackStatusHandler(testStatus);

  QLabel *currentTrackInfoLabel = mainWindow->findChild<QLabel *>("label_current_track_info");
  QLabel *elapsedTimeLabel = mainWindow->findChild<QLabel *>("label_elapsed_time");
  QLabel *durationLabel = mainWindow->findChild<QLabel *>("label_duration");
  QSlider *progressBar = mainWindow->findChild<QSlider *>("slider_progress");
  QSlider *volumeSlider = mainWindow->findChild<QSlider *>("slider_volume");
  QPushButton *playPauseButton = mainWindow->findChild<QPushButton *>("btn_play_pause");

  ASSERT_NE(currentTrackInfoLabel, nullptr);
  ASSERT_NE(elapsedTimeLabel, nullptr);
  ASSERT_NE(durationLabel, nullptr);
  ASSERT_NE(progressBar, nullptr);
  ASSERT_NE(volumeSlider, nullptr);
  ASSERT_NE(playPauseButton, nullptr);

  EXPECT_EQ(currentTrackInfoLabel->text().toStdString(), "Test Song Title");
  EXPECT_EQ(elapsedTimeLabel->text().toStdString(), "65");
  EXPECT_EQ(durationLabel->text().toStdString(), "240");
  EXPECT_EQ(progressBar->value(), testStatus.mElapsedTime / 1000);
  EXPECT_EQ(progressBar->maximum(), testStatus.mpSong->GetDuration() / 1000);
  EXPECT_EQ(playPauseButton->text().toStdString(), "Pause");
}
