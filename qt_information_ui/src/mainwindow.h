#ifndef QT_INFORMATION_UI_SRC_MAINWINDOW_H_
#define QT_INFORMATION_UI_SRC_MAINWINDOW_H_

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QThread>
#include <memory>

#include "media_client.h"
#include "ui_car.h"

namespace qt_information_ui
{
  class MainWindow : public QMainWindow
  {
    Q_OBJECT

  public:
    explicit MainWindow(std::shared_ptr<MediaClientInterface> client, QWidget *parent = nullptr);
    virtual ~MainWindow() = default;

  private slots:
    void OnBtnMediaClicked();
    void OnBtnPlayPauseClicked();
    void OnListPlaylistItemClicked(QListWidgetItem *item);
    void OnBtnNextClicked();
    void OnBtnPreviousClicked();
    void OnSliderVolumeChanged();
    void HandleMediaStatusUpdate(const PlaybackStatus &status);

  private:
    std::unique_ptr<Ui::MainWindow> mpUi;
    std::shared_ptr<MediaClientInterface> mpMediaClient;
    std::vector<Song> mSongs{};
    std::string mSelectedSongTitle{""};
    std::string mPrevSelectedSongTitle{""};
    PlaybackStatus mCurrentStatus{};

    void InitializeGrpcClients();
    bool LoadPlaylist();
  };
} // namespace qt_information_ui
#endif // QT_INFORMATION_UI_SRC_MAINWINDOW_H_