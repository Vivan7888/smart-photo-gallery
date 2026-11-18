#include "PhotoRepository.h"

#include <QDateTime>
#include <QCryptographicHash>
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
    album_tree_init(&m_albumTree, "All Photos");
    QDir().mkpath(activeDirectory());
}

PhotoRepository::~PhotoRepository() { album_tree_clear(&m_albumTree);
    photo_list_clear(&m_list); }

void PhotoRepository::copyText(char *destination, int length, const QString &value) {
    const QByteArray bytes = value.toUtf8();
    std::strncpy(destination, bytes.constData(), static_cast<size_t>(length - 1));
    destination[length - 1] = '\0';
}

bool PhotoRepository::load(QString *error) {
photo_list_clear(&m_list);
album_tree_clear(&m_albumTree);
album_tree_init(&m_albumTree, "All Photos");
bool metadataMigrated = false;

QFile file(metadataPath());
if (!file.exists()) return true;
if (!file.open(QIODevice::ReadOnly)) {
    if (error) *error = "Cannot open metadata file.";
    return false;
}
QJsonParseError parseError;
const QJsonDocument document =
    QJsonDocument::fromJson(file.readAll(), &parseError);
if (parseError.error != QJsonParseError::NoError ||
    !document.isArray()) {
    if (error) *error = "Metadata file is not valid JSON.";
    return false;
}
for (const QJsonValue &value : document.array()) {
    const QJsonObject object = value.toObject();
    PhotoRecord record{};
    record.id = object.value("id").toInt();
    copyText(record.name, PHOTO_NAME_LENGTH,
             object.value("name").toString());
    copyText(record.file_path, PHOTO_PATH_LENGTH,
             object.value("filePath").toString());
    copyText(record.date_added, PHOTO_DATE_LENGTH,
             object.value("dateAdded").toString());
    copyText(record.archive_path, PHOTO_PATH_LENGTH,
             object.value("archivePath").toString());
    copyText(record.album_name, PHOTO_ALBUM_LENGTH,
             object.value("albumName").toString("All Photos"));
    copyText(record.content_hash, PHOTO_HASH_LENGTH,
             object.value("contentHash").toString());
    record.original_size =
        object.value("originalSize").toVariant().toLongLong();
    record.stored_size =
        object.value("storedSize").toVariant().toLongLong();
    record.archived = object.value("archived").toBool() ? 1 : 0;

    // Metadata created before duplicate detection has no hash. Backfill it for
    // active photos so duplicates are still detected after upgrading the app.
    if (record.content_hash[0] == '\0' && !record.archived) {
        const QString hash = calculateFileHash(QString::fromUtf8(record.file_path), nullptr);
        if (!hash.isEmpty()) {
            copyText(record.content_hash, PHOTO_HASH_LENGTH, hash);
            metadataMigrated = true;
        }
    }
    PhotoRecord *addedRecord = photo_list_add(&m_list, &record);
    if (addedRecord != nullptr) {
        album_tree_add_photo(
            &m_albumTree,
            addedRecord->album_name,
            addedRecord->id
        );
    }
}
if (metadataMigrated && !save(error)) return false;
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
        object.insert("contentHash", QString::fromUtf8(record->content_hash));
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

    const QString contentHash = calculateFileHash(sourcePath, error);
    if (contentHash.isEmpty()) return nullptr;

    PhotoRecord *duplicate = findByContentHash(contentHash);
    if (duplicate != nullptr) {
        if (error) {
            *error = QString("Duplicate photo detected. It is already stored as ID %1 (%2).")
                         .arg(duplicate->id)
                         .arg(QString::fromUtf8(duplicate->name));
        }
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
    copyText(record.content_hash, PHOTO_HASH_LENGTH, contentHash);
    record.original_size = QFileInfo(managedPath).size();
    record.stored_size = record.original_size;
    record.archived = 0;

    PhotoRecord *added = photo_list_add(&m_list, &record);
    if (added == nullptr) {
        QFile::remove(managedPath);
        if (error) *error = "Not enough memory to add the photo record.";
        return nullptr;
    }
    album_tree_add_photo(
    &m_albumTree,
    added->album_name,
    added->id
);
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

bool PhotoRepository::removeRecord(int id) {
    PhotoRecord *record = photo_list_find(&m_list, id);

    if (record == nullptr) {
        return false;
    }

    album_tree_remove_photo(
        &m_albumTree,
        record->album_name,
        record->id
    );

    return photo_list_remove(&m_list, id) != 0;
}

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

QString PhotoRepository::calculateFileHash(const QString &path, QString *error) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) *error = "The selected image could not be read for duplicate checking.";
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    while (!file.atEnd()) {
        const QByteArray chunk = file.read(64 * 1024);
        if (chunk.isEmpty() && file.error() != QFileDevice::NoError) {
            if (error) *error = "The selected image could not be read completely for duplicate checking.";
            return {};
        }
        hash.addData(chunk);
    }
    return QString::fromLatin1(hash.result().toHex());
}

PhotoRecord *PhotoRepository::findByContentHash(const QString &hash) const {
    for (PhotoRecord *record = m_list.head; record != nullptr; record = record->next) {
        if (hash == QString::fromUtf8(record->content_hash)) return record;
    }
    return nullptr;
}
