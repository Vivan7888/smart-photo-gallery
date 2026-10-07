#ifndef PHOTO_H
#define PHOTO_H

#include <QString>
#include <QtGlobal>

extern "C" {
#include "photo_ds.h"
}

class Photo {
public:
    explicit Photo(const PhotoRecord &record);

    int id() const;
    QString name() const;
    QString dateAdded() const;
    qint64 originalSize() const;
    qint64 storedSize() const;
    bool isArchived() const;
    bool isOlderThanOneYear() const;

private:
    PhotoRecord m_record;
};

#endif
