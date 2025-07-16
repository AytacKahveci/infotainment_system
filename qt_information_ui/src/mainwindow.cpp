#include "mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QListWidgetItem>

namespace qt_information_ui
{
  MainWindow::MainWindow(std::shared_ptr<MediaClientInterface> client, QWidget *parent)
      : mpMediaClient(client),
        QMainWindow(parent),
        mpUi(std::make_unique<Ui::MainWindow>())
  {
    mpUi->setupUi(this);

    mpUi->stackedWidget->setCurrentWidget(mpUi->homePage);

    connect(mpUi->btn_media, &QPushButton::clicked, this, &MainWindow::OnBtnMediaClicked);
    connect(mpUi->btn_play_pause, &QPushButton::clicked, this, &MainWindow::OnBtnPlayPauseClicked);
    connect(mpUi->list_playlist, &QListWidget::itemClicked, this, &MainWindow::OnListPlaylistItemClicked);
    connect(mpUi->btn_next, &QPushButton::clicked, this, &MainWindow::OnBtnNextClicked);
    connect(mpUi->btn_previous, &QPushButton::clicked, this, &MainWindow::OnBtnPreviousClicked);
    connect(mpUi->slider_volume, &QSlider::valueChanged, this, &MainWindow::OnSliderVolumeChanged);

    InitializeGrpcClients();

    LoadPlaylist();
  }

  void MainWindow::InitializeGrpcClients()
  {
    if (!mpMediaClient)
    {
      QMessageBox::critical(this, "gRPC Error", "Failed to create Media gRPC client");
      qFatal("Failed to create Media gRPC client");
    }

    mpMediaClient->SetHandler([this](const PlaybackStatus &status)
                              { HandleMediaStatusUpdate(status); });
  }

  bool MainWindow::LoadPlaylist()
  {
    if (!mpMediaClient)
    {
      return false;
    }

    const auto &songs = mpMediaClient->GetSongs();
    if (songs.size() > 0)
    {
      mSongs = songs;
      mpUi->list_playlist->clear();
      for (const auto &song : songs)
      {
        mpUi->list_playlist->addItem(QString::fromStdString(song.GetTrackTitle()));
      }
      qDebug() << "Playlist loaded from MediaService";
      return true;
    }
    else
    {
      QMessageBox::warning(this, "gRPC Error", "Failed to get playlist from MediaService");
      qWarning() << "Failed to get playlist";
      return false;
    }
  }

  void MainWindow::OnBtnMediaClicked()
  {
    mpUi->stackedWidget->setCurrentWidget(mpUi->mediaPage);
    qDebug() << "Switched to Media Page.";
  }

  void MainWindow::OnBtnPlayPauseClicked()
  {
    switch (mCurrentStatus.mState)
    {
    case PlaybackState::PLAYING:
    {
      mpMediaClient->PauseSong();
      break;
    }
    case PlaybackState::PAUSED:
    {
      mpMediaClient->PauseSong(false);
      break;
    }
    case PlaybackState::STOPPED:
    default:
    {
      if (mPrevSelectedSongTitle == mSelectedSongTitle)
      {
        return;
      }

      const auto it = std::find_if(mSongs.begin(), mSongs.end(), [this](const qt_information_ui::Song &song)
                                   { return song.GetTrackTitle() == mSelectedSongTitle; });
      if (it == mSongs.end())
      {
        std::cout << "Song couldn't found: " << mSelectedSongTitle << std::endl;
        return;
      }

      if (mpMediaClient->PlaySong(*it))
      {
        std::cout << "Play successfull" << std::endl;
        mPrevSelectedSongTitle = mSelectedSongTitle;
        mpUi->btn_play_pause->setText("Pause");
      }
      break;
    }
    }
  }

  void MainWindow::OnListPlaylistItemClicked(QListWidgetItem *item)
  {
    if (item)
    {
      mSelectedSongTitle = item->text().toStdString();
    }
  }

  void MainWindow::OnBtnNextClicked()
  {
    mpMediaClient->PlayNext();
  }

  void MainWindow::OnBtnPreviousClicked()
  {
    mpMediaClient->PlayPrevious();
  }

  void MainWindow::OnSliderVolumeChanged()
  {
    const auto volume = mpUi->slider_volume->value();
    if (!mpMediaClient->SetVolume(volume))
    {
      std::cout << "Volume couldn't set" << std::endl;
    }
  }

  void MainWindow::HandleMediaStatusUpdate(const PlaybackStatus &status)
  {
    mCurrentStatus = status;
    switch (status.mState)
    {
    case PlaybackState::PAUSED:
      mpUi->label_now_playing_title->setText("Paused:");
      mpUi->btn_play_pause->setText("Play");
      break;
    case PlaybackState::STOPPED:
      mpUi->label_now_playing_title->setText("Stopped:");
      mpUi->btn_play_pause->setText("Play");
      break;
    case PlaybackState::PLAYING:
    default:
      mpUi->label_now_playing_title->setText("Now Playing:");
      mpUi->btn_play_pause->setText("Pause");
      break;
    }

    if (status.mpSong)
    {
      mpUi->label_current_track_info->setText(QString::fromStdString(status.mpSong->GetTrackTitle()));
      mpUi->slider_progress->setValue(status.mElapsedTime / 1000);
      mpUi->slider_progress->setMinimum(0);
      mpUi->slider_progress->setMaximum(status.mpSong->GetDuration() / 1000);
      mpUi->label_duration->setText(QString().setNum(status.mpSong->GetDuration() / 1000));
      mpUi->label_elapsed_time->setText(QString().setNum(status.mElapsedTime / 1000));
    }
  }

} // namespace qt_information_ui