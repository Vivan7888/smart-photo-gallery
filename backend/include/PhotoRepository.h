#ifndef PHOTO_REPOSITORY_H
#define PHOTO_REPOSITORY_H

#include <QString>
#include <QVector>

extern "C" {
#include "photo_ds.h"
#include "album_tree.h"
}

class PhotoRepository {
public:
    explicit PhotoRepository(const QString &storageRoot);
    ~PhotoRepository();

    bool load(QString *error = nullptr);
    bool save(QString *error = nullptr) const;
    PhotoRecord *importPhoto(const QString &sourcePath, QString *error = nullptr);
    PhotoRecord *find(int id);
    QVector<PhotoRecord *> records() const;
    bool removeRecord(int id);
    void sortByColumn(int column);
    int count() const;
    qint64 totalStoredBytes() const;

    QString storageRoot() const;
    QString activeDirectory() const;
    QString metadataPath() const;

private:
    PhotoList m_list;
    Albumtree m_albumTree;
    QString m_storageRoot;
    static void copyText(char *destination, int length, const QString &value);
};

#endif
