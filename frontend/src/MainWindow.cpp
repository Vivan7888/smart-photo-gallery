#include "MainWindow.h"

#include "Photo.h"

#include <QColor>
#include <QDate>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_repository(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)),
      m_storageManager(m_repository) {
    action_stack_init(&m_actions);
    buildUi();

    QString error;
    if (!m_repository.load(&error)) {
        QMessageBox::warning(this, "Smart Photo Gallery", error);
    }
    refreshTable();
}

MainWindow::~MainWindow() { action_stack_clear(&m_actions); }

void MainWindow::buildUi() {
    setWindowTitle("Smart Photo Gallery");
    resize(1200, 760);
    setMinimumSize(860, 580);
    setStyleSheet(
        "QMainWindow { background: #0b1220; }"
        "QWidget#galleryRoot { background: #0b1220; }"
        "QFrame#heroPanel { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #172554, stop:0.48 #172554, stop:1 #0f766e); border: 1px solid #334a81; border-radius: 18px; }"
        "QFrame#summaryCard { background: #121c2f; border: 1px solid #263652; border-radius: 14px; }"
        "QFrame#controlPanel { background: #101a2c; border: 1px solid #263652; border-radius: 14px; }"
        "QPushButton { background: #24324b; color: #dbeafe; border: 1px solid #354867; border-radius: 9px; padding: 9px 13px; font-weight: 600; }"
        "QPushButton:hover { background: #334665; border-color: #5b7baa; }"
        "QPushButton#primaryButton { background: #2563eb; color: white; border-color: #3b82f6; }"
        "QPushButton#primaryButton:hover { background: #1d4ed8; }"
        "QPushButton#archiveButton { color: #fde68a; }"
        "QPushButton#dangerButton { background: #3b1b2c; color: #fecdd3; border-color: #7f3148; }"
        "QPushButton#dangerButton:hover { background: #5b1f37; }"
        "QLineEdit { background: #0b1322; color: #e2e8f0; border: 1px solid #354867; border-radius: 9px; padding: 9px 12px; selection-background-color: #2563eb; }"
        "QLineEdit:focus { border: 1px solid #60a5fa; }"
        "QTableWidget { background: #101a2c; alternate-background-color: #131f34; color: #e2e8f0; border: 1px solid #263652; border-radius: 14px; gridline-color: #1d2b43; selection-background-color: #1e3a5f; selection-color: #ffffff; }"
        "QHeaderView::section { background: #16233a; color: #93c5fd; border: none; border-bottom: 1px solid #334155; padding: 11px 9px; font-weight: 700; }"
        "QTableCornerButton::section { background: #16233a; border: none; }"
        "QStatusBar { background: #0b1220; color: #94a3b8; border-top: 1px solid #1e293b; }");

    auto *central = new QWidget(this);
    central->setObjectName("galleryRoot");
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(28, 26, 28, 22);
    mainLayout->setSpacing(16);

    auto *heroPanel = new QFrame(central);
    heroPanel->setObjectName("heroPanel");
    auto *heroLayout = new QVBoxLayout(heroPanel);
    heroLayout->setContentsMargins(24, 20, 24, 20);
    heroLayout->setSpacing(5);
    auto *eyebrow = new QLabel("LOCAL-FIRST PHOTO ARCHIVE", heroPanel);
    eyebrow->setStyleSheet("color: #a5f3fc; font-size: 11px; font-weight: 700; letter-spacing: 1px;");
    auto *title = new QLabel("Smart Photo Gallery", heroPanel);
    title->setStyleSheet("font-size: 30px; font-weight: 800; color: white;");
    auto *subtitle = new QLabel("Keep memories on your device. Archive older photos, recover them instantly when you need them.", heroPanel);
    subtitle->setStyleSheet("color: #dbeafe; font-size: 14px;");
    heroLayout->addWidget(eyebrow);
    heroLayout->addWidget(title);
    heroLayout->addWidget(subtitle);
    mainLayout->addWidget(heroPanel);

    auto *summaryLayout = new QHBoxLayout;
    summaryLayout->setSpacing(12);
    auto createCard = [central](const QString &caption, const QString &accent, QLabel **valueLabel) {
        auto *card = new QFrame(central);
        card->setObjectName("summaryCard");
        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(18, 15, 18, 15);
        layout->setSpacing(4);
        auto *captionLabel = new QLabel(caption, card);
        captionLabel->setStyleSheet("color: #94a3b8; font-weight: 600;");
        *valueLabel = new QLabel("0", card);
        (*valueLabel)->setStyleSheet("font-size: 25px; font-weight: 800; color: " + accent + ";");
        layout->addWidget(captionLabel);
        layout->addWidget(*valueLabel);
        return card;
    };
    summaryLayout->addWidget(createCard("PHOTOS IN LIBRARY", "#93c5fd", &m_photoCountLabel));
    summaryLayout->addWidget(createCard("MANAGED STORAGE", "#5eead4", &m_storageLabel));
    summaryLayout->addWidget(createCard("READY TO ARCHIVE", "#fcd34d", &m_oldPhotoLabel));
    mainLayout->addLayout(summaryLayout);

    auto *controlPanel = new QFrame(central);
    controlPanel->setObjectName("controlPanel");
    auto *controls = new QHBoxLayout(controlPanel);
    controls->setContentsMargins(12, 12, 12, 12);
    controls->setSpacing(9);
    auto *addButton = new QPushButton("+  Add photos", controlPanel);
    addButton->setObjectName("primaryButton");
    auto *archiveButton = new QPushButton("Archive selected", controlPanel);
    archiveButton->setObjectName("archiveButton");
    auto *archiveOlderButton = new QPushButton("Archive eligible", controlPanel);
    archiveOlderButton->setObjectName("archiveButton");
    auto *previewButton = new QPushButton("Preview", controlPanel);
    auto *undoButton = new QPushButton("Undo archive", controlPanel);
    auto *deleteButton = new QPushButton("Delete", controlPanel);
    deleteButton->setObjectName("dangerButton");
    m_searchInput = new QLineEdit(controlPanel);
    m_searchInput->setPlaceholderText("Search photos or dates...");
    m_searchInput->setMinimumWidth(210);
    addButton->setToolTip("Import photos into local managed storage");
    archiveButton->setToolTip("Archive the selected photo, regardless of its date");
    archiveOlderButton->setToolTip("Archive every photo that is at least one year old");
    previewButton->setToolTip("Open the selected image; archived files are restored temporarily");
    undoButton->setToolTip("Restore the most recently archived photo");

    controls->addWidget(addButton);
    controls->addWidget(archiveButton);
    controls->addWidget(archiveOlderButton);
    controls->addWidget(previewButton);
    controls->addWidget(undoButton);
    controls->addWidget(deleteButton);
    controls->addStretch();
    controls->addWidget(m_searchInput, 1);
    mainLayout->addWidget(controlPanel);

    m_photoTable = new QTableWidget(0, 6, central);
    m_photoTable->setHorizontalHeaderLabels({"ID", "Photo", "Date", "Original size", "Currently used", "Status"});
    m_photoTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_photoTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_photoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_photoTable->setAlternatingRowColors(true);
    m_photoTable->setShowGrid(false);
    m_photoTable->setFocusPolicy(Qt::NoFocus);
    m_photoTable->verticalHeader()->setDefaultSectionSize(42);
    m_photoTable->verticalHeader()->setVisible(false);
    m_photoTable->horizontalHeader()->setStretchLastSection(true);
    m_photoTable->setColumnWidth(0, 55);
    m_photoTable->setColumnWidth(1, 250);
    m_photoTable->setColumnWidth(2, 110);
    mainLayout->addWidget(m_photoTable, 1);

    setCentralWidget(central);
    statusBar()->showMessage("Ready — your photos stay local.");

    connect(addButton, &QPushButton::clicked, this, [this] { importPhotos(); });
    connect(previewButton, &QPushButton::clicked, this, [this] { previewSelectedPhoto(); });
    connect(archiveButton, &QPushButton::clicked, this, [this] { archiveSelectedPhoto(); });
    connect(archiveOlderButton, &QPushButton::clicked, this, [this] { archiveOlderPhotos(); });
    connect(undoButton, &QPushButton::clicked, this, [this] { undoLastArchive(); });
    connect(deleteButton, &QPushButton::clicked, this, [this] { deleteSelectedPhoto(); });
    connect(m_searchInput, &QLineEdit::textChanged, this, [this] { refreshTable(); });
    connect(m_photoTable->horizontalHeader(), &QHeaderView::sectionClicked, this, [this](int column) {
        m_repository.sortByColumn(column);
        refreshTable();
    });
}

QString MainWindow::formatBytes(qint64 bytes) {
    const QStringList units = {"B", "KB", "MB", "GB"};
    double value = static_cast<double>(bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < units.size() - 1) { value /= 1024.0; ++unit; }
    return QString::number(value, 'f', unit == 0 ? 0 : 2) + " " + units[unit];
}

void MainWindow::updateSummary() {
    int eligible = 0;
    for (PhotoRecord *record : m_repository.records()) {
        if (!record->archived && m_storageManager.isOlderThanOneYear(*record)) ++eligible;
    }
    m_photoCountLabel->setText(QString::number(m_repository.count()));
    m_storageLabel->setText(formatBytes(m_repository.totalStoredBytes()));
    m_oldPhotoLabel->setText(QString::number(eligible));
}

void MainWindow::refreshTable() {
    const QString filter = m_searchInput ? m_searchInput->text().trimmed() : QString();
    m_photoTable->setRowCount(0);
    for (PhotoRecord *record : m_repository.records()) {
        const Photo photo(*record);
        if (!filter.isEmpty() && !photo.name().contains(filter, Qt::CaseInsensitive) && !photo.dateAdded().contains(filter)) continue;
        const int row = m_photoTable->rowCount();
        m_photoTable->insertRow(row);
        auto *idItem = new QTableWidgetItem(QString::number(photo.id()));
        idItem->setData(Qt::UserRole, photo.id());
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setForeground(QColor("#93c5fd"));
        m_photoTable->setItem(row, 0, idItem);
        auto *nameItem = new QTableWidgetItem(photo.name());
        nameItem->setToolTip(QString::fromUtf8(record->file_path));
        m_photoTable->setItem(row, 1, nameItem);
        auto *dateItem = new QTableWidgetItem(photo.dateAdded());
        dateItem->setTextAlignment(Qt::AlignCenter);
        m_photoTable->setItem(row, 2, dateItem);
        auto *originalSizeItem = new QTableWidgetItem(formatBytes(photo.originalSize()));
        originalSizeItem->setTextAlignment(Qt::AlignCenter);
        m_photoTable->setItem(row, 3, originalSizeItem);
        auto *storedSizeItem = new QTableWidgetItem(formatBytes(photo.storedSize()));
        storedSizeItem->setTextAlignment(Qt::AlignCenter);
        m_photoTable->setItem(row, 4, storedSizeItem);
        auto *statusItem = new QTableWidgetItem(photo.isArchived() ? "ARCHIVED" : "ACTIVE");
        statusItem->setTextAlignment(Qt::AlignCenter);
        statusItem->setForeground(photo.isArchived() ? QColor("#fcd34d") : QColor("#5eead4"));
        m_photoTable->setItem(row, 5, statusItem);
    }
    updateSummary();
}

int MainWindow::selectedPhotoId() const {
    const int row = m_photoTable->currentRow();
    if (row < 0 || !m_photoTable->item(row, 0)) return -1;
    return m_photoTable->item(row, 0)->data(Qt::UserRole).toInt();
}

void MainWindow::importPhotos() {
    // Use Qt's own picker instead of the macOS native picker. This prevents the
    // native dialog from appearing behind the application on some systems.
    QFileDialog picker(this, "Choose photos");
    picker.setFileMode(QFileDialog::ExistingFiles);
    picker.setNameFilter("Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp *.heic *.HEIC);;All files (*)");
    picker.setOption(QFileDialog::DontUseNativeDialog, true);
    if (picker.exec() != QDialog::Accepted) return;
    const QStringList paths = picker.selectedFiles();
    if (paths.isEmpty()) return;

    int imported = 0;
    QStringList failures;
    for (const QString &path : paths) {
        QString error;
        if (m_repository.importPhoto(path, &error)) ++imported;
        else failures.append(QFileInfo(path).fileName() + ": " + error);
    }
    refreshTable();
    statusBar()->showMessage(QString("Imported %1 photo(s).").arg(imported), 5000);
    if (!failures.isEmpty()) QMessageBox::warning(this, "Some photos were not imported", failures.join("\n"));
}

void MainWindow::previewSelectedPhoto() {
    const int id = selectedPhotoId();
    PhotoRecord *record = m_repository.find(id);
    if (!record) {
        QMessageBox::information(this, "Select a photo", "Select a photo from the table first.");
        return;
    }
    bool temporary = false;
    QString error;
    const QString path = m_storageManager.previewPath(*record, &temporary, &error);
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Preview unavailable", error);
        return;
    }

    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        if (temporary) QFile::remove(path);
        QMessageBox::warning(this, "Preview unavailable", "The selected file is not a supported image.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle(QString::fromUtf8(record->name));
    dialog.resize(900, 700);
    auto *layout = new QVBoxLayout(&dialog);
    auto *image = new QLabel(&dialog);
    image->setAlignment(Qt::AlignCenter);
    image->setPixmap(pixmap.scaled(850, 580, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(image, 1);
    auto *caption = new QLabel(
        QString("%1 • %2 • %3")
            .arg(QString::fromUtf8(record->name), QString::fromUtf8(record->date_added), record->archived ? "Temporarily decompressed" : "Active"),
        &dialog);
    caption->setAlignment(Qt::AlignCenter);
    layout->addWidget(caption);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dialog);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    dialog.exec();
    if (temporary) QFile::remove(path);
}

void MainWindow::archiveSelectedPhoto() {
    PhotoRecord *record = m_repository.find(selectedPhotoId());
    if (!record) {
        QMessageBox::information(this, "Select a photo", "Select a photo from the table first.");
        return;
    }
    QString error;
    if (!m_storageManager.archivePhoto(*record,true, &error)) {
        QMessageBox::information(this, "Photo not archived", error);
        return;
    }
    action_stack_push(&m_actions, record->id, ACTION_ARCHIVE);
    refreshTable();
    statusBar()->showMessage("Photo archived locally.", 5000);
}

void MainWindow::archiveOlderPhotos() {
    int eligible = 0;
    int archived = 0;
    for (PhotoRecord *record : m_repository.records()) {
        if (!record->archived && m_storageManager.isOlderThanOneYear(*record)) {
            ++eligible;
            QString error;
            if (m_storageManager.archivePhoto(*record,false, &error)) {
                action_stack_push(&m_actions, record->id, ACTION_ARCHIVE);
                ++archived;
            }
        }
    }
    refreshTable();
    QMessageBox::information(this, "Archive result",
        QString("Eligible photos: %1\nArchived: %2\n\nOnly files that become smaller after lossless compression are archived.")
            .arg(eligible).arg(archived));
}

void MainWindow::undoLastArchive() {
    int photoId = -1;
    int actionType = 0;
    if (!action_stack_pop(&m_actions, &photoId, &actionType)) {
        QMessageBox::information(this, "Nothing to undo", "There is no archive action to undo.");
        return;
    }
    PhotoRecord *record = m_repository.find(photoId);
    QString error;
    if (actionType != ACTION_ARCHIVE || !record || !m_storageManager.restorePhoto(*record, &error)) {
        QMessageBox::warning(this, "Undo failed", error.isEmpty() ? "The photo cannot be restored." : error);
        return;
    }
    refreshTable();
    statusBar()->showMessage("Archive action undone.", 5000);
}

void MainWindow::deleteSelectedPhoto() {
    PhotoRecord *record = m_repository.find(selectedPhotoId());
    if (!record) {
        QMessageBox::information(this, "Select a photo", "Select a photo from the table first.");
        return;
    }
    const auto decision = QMessageBox::warning(this, "Delete managed photo",
        "This removes the application's managed copy and its gallery record. Your original imported source file is not changed. Continue?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (decision != QMessageBox::Yes) return;

    QString error;
    if (!m_storageManager.deleteStoredFile(*record, &error) || !m_repository.removeRecord(record->id) || !m_repository.save(&error)) {
        QMessageBox::warning(this, "Delete failed", error.isEmpty() ? "The photo could not be removed." : error);
        return;
    }
    refreshTable();
    statusBar()->showMessage("Managed photo removed.", 5000);
}
