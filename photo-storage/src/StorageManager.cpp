#include "StorageManager.h"

#include "PhotoRepository.h"

#include <QByteArray>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>

StorageManager::StorageManager(PhotoRepository &repository) : m_repository(repository) {
    QDir().mkpath(archiveDirectory());
}

QString StorageManager::archiveDirectory() const {
    return QDir(m_repository.storageRoot()).filePath("archives");
}

bool StorageManager::isOlderThanOneYear(const PhotoRecord &record) const {
    const QString dateString = QString::fromUtf8(record.date_added);
    const QDate addedDate = QDate::fromString(dateString, Qt::ISODate);

    if (!addedDate.isValid()) {
        return false;
    }

    const QDate oneYearAgo = QDate::currentDate().addYears(-1);
    return addedDate <= oneYearAgo;
}

bool StorageManager::archivePhoto(PhotoRecord &record, bool manualOverride, QString *error) {
    if (record.archived) {
        if (error) *error = "This photo is already archived.";
        return false;
    }
    if (!manualOverride && !isOlderThanOneYear(record)) {
        if (error) *error = "Only photos at least one year old can be archived automatically.";
        return false;
    }

    QFile source(QString::fromUtf8(record.file_path));
    if (!source.open(QIODevice::ReadOnly)) {
        if (error) *error = "The managed photo file is missing.";
        return false;
    }
    const QByteArray originalBytes = source.readAll();
    source.close();
    const QByteArray compressedBytes = qCompress(originalBytes, 9);
    if (compressedBytes.size() >= originalBytes.size()) {
        if (error) *error = "This image is already efficiently compressed. Keeping the original is safer and uses less space.";
        return false;
    }

    QDir().mkpath(archiveDirectory());
    const QString archivePath = QDir(archiveDirectory()).filePath(QString::number(record.id) + ".spg");
    QSaveFile archiveFile(archivePath);
    if (!archiveFile.open(QIODevice::WriteOnly) || archiveFile.write(compressedBytes) != compressedBytes.size() || !archiveFile.commit()) {
        if (error) *error = "Could not create the archive file.";
        return false;
    }

    if (!QFile::remove(QString::fromUtf8(record.file_path))) {
        QFile::remove(archivePath);
        if (error) *error = "Archive was created, but the active managed copy could not be removed.";
        return false;
    }

    const QByteArray pathBytes = archivePath.toUtf8();
    qstrncpy(record.archive_path, pathBytes.constData(), PHOTO_PATH_LENGTH);
    record.archived = 1;
    record.stored_size = compressedBytes.size();
    if (!m_repository.save(error)) return false;
    return true;
}

bool StorageManager::restorePhoto(PhotoRecord &record, QString *error) {
    if (!record.archived) {
        if (error) *error = "This photo is not archived.";
        return false;
    }
    QFile archive(QString::fromUtf8(record.archive_path));
    if (!archive.open(QIODevice::ReadOnly)) {
        if (error) *error = "The archive file is missing.";
        return false;
    }
    const QByteArray originalBytes = qUncompress(archive.readAll());
    if (originalBytes.isEmpty()) {
        if (error) *error = "The archive could not be decompressed.";
        return false;
    }

    QDir().mkpath(m_repository.activeDirectory());
    QSaveFile activeFile(QString::fromUtf8(record.file_path));
    if (!activeFile.open(QIODevice::WriteOnly) || activeFile.write(originalBytes) != originalBytes.size() || !activeFile.commit()) {
        if (error) *error = "Could not restore the active photo file.";
        return false;
    }
    QFile::remove(QString::fromUtf8(record.archive_path));
    record.archived = 0;
    record.archive_path[0] = '\0';
    record.stored_size = originalBytes.size();
    return m_repository.save(error);
}

QString StorageManager::previewPath(const PhotoRecord &record, bool *isTemporary, QString *error) const {
    *isTemporary = false;
    if (!record.archived) {
        const QString activePath = QString::fromUtf8(record.file_path);
        if (!QFileInfo::exists(activePath)) {
            if (error) *error = "The managed photo file is missing.";
            return {};
        }
        return activePath;
    }

    QFile archive(QString::fromUtf8(record.archive_path));
    if (!archive.open(QIODevice::ReadOnly)) {
        if (error) *error = "The archive file is missing.";
        return {};
    }
    const QByteArray originalBytes = qUncompress(archive.readAll());
    if (originalBytes.isEmpty()) {
        if (error) *error = "The archive could not be decompressed.";
        return {};
    }

    const QString temporaryRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(temporaryRoot);
    const QString temporaryPath = QDir(temporaryRoot).filePath(
        QString("smart-gallery-%1-%2").arg(record.id).arg(QString::fromUtf8(record.name)));
    QSaveFile temporaryFile(temporaryPath);
    if (!temporaryFile.open(QIODevice::WriteOnly) || temporaryFile.write(originalBytes) != originalBytes.size() || !temporaryFile.commit()) {
        if (error) *error = "Could not create the temporary preview file.";
        return {};
    }
    *isTemporary = true;
    return temporaryPath;
}

bool StorageManager::deleteStoredFile(const PhotoRecord &record, QString *error) const {
    const QString target = record.archived ? QString::fromUtf8(record.archive_path) : QString::fromUtf8(record.file_path);
    if (target.isEmpty() || !QFileInfo::exists(target)) return true;
    if (!QFile::remove(target)) {
        if (error) *error = "Could not remove the managed photo file.";
        return false;
    }
    return true;
}
