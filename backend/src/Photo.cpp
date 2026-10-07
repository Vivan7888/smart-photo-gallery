#include "Photo.h"

#include <QDate>

Photo::Photo(const PhotoRecord &record) : m_record(record) { m_record.next = nullptr; }

int Photo::id() const { return m_record.id; }
QString Photo::name() const { return QString::fromUtf8(m_record.name); }
QString Photo::dateAdded() const { return QString::fromUtf8(m_record.date_added); }
qint64 Photo::originalSize() const { return m_record.original_size; }
qint64 Photo::storedSize() const { return m_record.stored_size; }
bool Photo::isArchived() const { return m_record.archived != 0; }

bool Photo::isOlderThanOneYear() const {
    const QDate date = QDate::fromString(dateAdded(), Qt::ISODate);
    return date.isValid() && date <= QDate::currentDate().addYears(-1);
}
