#include "PhotoRepository.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <cstring>

PhotoRepository::PhotoRepository(const QString &storageRoot) : m_storageRoot(storageRoot) {
    photo_list_init(&m_list);
    QDir().mkpath(activeDirectory());
}

PhotoRepository::~PhotoRepository() { photo_list_clear(&m_list); }

void PhotoRepository::copyText(char *destination, int length, const QString &value) {
    const QByteArray bytes = value.toUtf8();
    std::strncpy(destination, bytes.constData(), static_cast<size_t>(length - 1));
    destination[length - 1] = '\0';
}

bool PhotoRepository::load(QString *error) {
    photo_list_clear(&m_list);
    QFile file(metadataPath());
    if (!file.exists()) return true;
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = "Cannot open metadata file.";
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isArray()) {
        if (error) *error = "Metadata file is not valid JSON.";
        return false;
    }

    for (const QJsonValue &value : document.array()) {
        const QJsonObject object = value.toObject();
        PhotoRecord record{};
        record.id = object.value("id").toInt();
        copyText(record.name, PHOTO_NAME_LENGTH, object.value("name").toString());
        copyText(record.file_path, PHOTO_PATH_LENGTH, object.value("filePath").toString());
        copyText(record.date_added, PHOTO_DATE_LENGTH, object.value("dateAdded").toString());
        copyText(record.archive_path, PHOTO_PATH_LENGTH, object.value("archivePath").toString());
        copyText(record.album_name, PHOTO_ALBUM_LENGTH, object.value("albumName").toString("All Photos"));
        record.original_size = object.value("originalSize").toVariant().toLongLong();
        record.stored_size = object.value("storedSize").toVariant().toLongLong();
        record.archived = object.value("archived").toBool() ? 1 : 0;
        photo_list_add(&m_list, &record);
    }
    return true;
}

bool PhotoRepository::save(QString *error) const {
    QJsonArray photos;
    for (PhotoRecord *record = m_list.head; record != nullptr; record = record->next) {
        QJsonObject object;
        object.insert("id", record->id);
        object.insert("name", QString::fromUtf8(record->name));
        object.insert("filePath", QString::fromUtf8(record->file_path));
        object.insert("dateAdded", QString::fromUtf8(record->date_added));
        object.insert("archivePath", QString::fromUtf8(record->archive_path));
        object.insert("albumName", QString::fromUtf8(record->album_name));
        object.insert("originalSize", static_cast<double>(record->original_size));
        object.insert("storedSize", static_cast<double>(record->stored_size));
        object.insert("archived", record->archived != 0);
        photos.append(object);
    }

    QDir().mkpath(m_storageRoot);
    QSaveFile file(metadataPath());
    if (!file.open(QIODevice::WriteOnly)) {
        if (error) *error = "Cannot write metadata file.";
        return false;
    }
    file.write(QJsonDocument(photos).toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        if (error) *error = "Cannot finish saving metadata.";
        return false;
    }
    return true;
}

PhotoRecord *PhotoRepository::importPhoto(const QString &sourcePath, QString *error) {
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        if (error) *error = "The selected image file does not exist.";
        return nullptr;
    }

    QDir().mkpath(activeDirectory());
    const QString safeName = QString::number(QDateTime::currentMSecsSinceEpoch()) + "_" + sourceInfo.fileName();
    const QString managedPath = QDir(activeDirectory()).filePath(safeName);
    if (!QFile::copy(sourcePath, managedPath)) {
        if (error) *error = "The app could not copy the image into its managed storage.";
        return nullptr;
    }

    PhotoRecord record{};
    copyText(record.name, PHOTO_NAME_LENGTH, sourceInfo.fileName());
    copyText(record.file_path, PHOTO_PATH_LENGTH, managedPath);
    copyText(record.date_added, PHOTO_DATE_LENGTH, sourceInfo.lastModified().date().toString(Qt::ISODate));
    copyText(record.album_name, PHOTO_ALBUM_LENGTH, "All Photos");
    record.original_size = QFileInfo(managedPath).size();
    record.stored_size = record.original_size;
    record.archived = 0;

    PhotoRecord *added = photo_list_add(&m_list, &record);
    if (added == nullptr) {
        QFile::remove(managedPath);
        if (error) *error = "Not enough memory to add the photo record.";
        return nullptr;
    }
    if (!save(error)) {
        photo_list_remove(&m_list, added->id);
        QFile::remove(managedPath);
        return nullptr;
    }
    return added;
}

PhotoRecord *PhotoRepository::find(int id) { return photo_list_find(&m_list, id); }

QVector<PhotoRecord *> PhotoRepository::records() const {
    QVector<PhotoRecord *> result;
    for (PhotoRecord *record = m_list.head; record != nullptr; record = record->next) result.append(record);
    return result;
}

bool PhotoRepository::removeRecord(int id) { return photo_list_remove(&m_list, id) != 0; }

void PhotoRepository::sortByColumn(int column) {
    if (column == 1) photo_list_sort_by_name(&m_list);
    else if (column == 2) photo_list_sort_by_date(&m_list);
    else if (column == 3) photo_list_sort_by_size(&m_list);
}

int PhotoRepository::count() const { return photo_list_count(&m_list); }

qint64 PhotoRepository::totalStoredBytes() const {
    qint64 total = 0;
    for (PhotoRecord *record = m_list.head; record != nullptr; record = record->next) total += record->stored_size;
    return total;
}

QString PhotoRepository::storageRoot() const { return m_storageRoot; }
QString PhotoRepository::activeDirectory() const { return QDir(m_storageRoot).filePath("active"); }
QString PhotoRepository::metadataPath() const { return QDir(m_storageRoot).filePath("metadata.json"); }
