#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

#include "PhotoRepository.h"
#include "StorageManager.h"

extern "C" {
#include "photo_ds.h"
}

class QLabel;
class QLineEdit;
class QTableWidget;

class MainWindow : public QMainWindow {
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void buildUi();
    void refreshTable();
    void updateSummary();
    int selectedPhotoId() const;
    void importPhotos();
    void previewSelectedPhoto();
    void archiveSelectedPhoto();
    void archiveOlderPhotos();
    void undoLastArchive();
    void deleteSelectedPhoto();
    static QString formatBytes(qint64 bytes);

    PhotoRepository m_repository;
    StorageManager m_storageManager;
    ActionStack m_actions;
    QLineEdit *m_searchInput = nullptr;
    QTableWidget *m_photoTable = nullptr;
    QLabel *m_photoCountLabel = nullptr;
    QLabel *m_storageLabel = nullptr;
    QLabel *m_oldPhotoLabel = nullptr;
};

#endif
