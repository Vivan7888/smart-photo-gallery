#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <QString>

extern "C" {
#include "photo_ds.h"
}

class PhotoRepository;

class StorageManager {
public:
    explicit StorageManager(PhotoRepository &repository);

    bool isOlderThanOneYear(const PhotoRecord &record) const;
    bool archivePhoto(PhotoRecord &record, QString *error = nullptr);
    bool restorePhoto(PhotoRecord &record, QString *error = nullptr);
    QString previewPath(const PhotoRecord &record, bool *isTemporary, QString *error = nullptr) const;
    bool deleteStoredFile(const PhotoRecord &record, QString *error = nullptr) const;
    QString archiveDirectory() const;

private:
    PhotoRepository &m_repository;
};

#endif
