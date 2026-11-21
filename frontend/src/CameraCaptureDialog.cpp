#include "CameraCaptureDialog.h"

#include <QCamera>
#include <QDateTime>
#include <QDir>
#include <QImageCapture>
#include <QImage>
#include <QLabel>
#include <QMediaCaptureSession>
#include <QMediaDevices>
#include <QPixmap>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVideoFrame>
#include <QVideoSink>

CameraCaptureDialog::CameraCaptureDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Capture a photo");
    setModal(true);
    resize(760, 590);
    setMinimumSize(520, 430);
    setStyleSheet(
        "QDialog { background: #f8f7fc; }"
        "QLabel#cameraTitle { color: #302142; font-size: 21px; font-weight: 800; }"
        "QLabel#cameraHint { color: #756b83; }"
        "QLabel#viewfinder { background: #201a2a; border: 1px solid #ddd3f5; border-radius: 12px; }"
        "QPushButton { background: #f3f0f8; color: #463c56; border: 1px solid #dfd7ea; border-radius: 9px; padding: 9px 16px; font-weight: 600; }"
        "QPushButton:hover { background: #ebe5f4; }"
        "QPushButton#captureButton { background: #7357c9; color: white; border-color: #7357c9; }"
        "QPushButton#captureButton:hover { background: #6044b5; }");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(10);

    auto *title = new QLabel("Take a photo", this);
    title->setObjectName("cameraTitle");
    auto *hint = new QLabel("Your photo will be imported directly into Smart Photo Gallery.", this);
    hint->setObjectName("cameraHint");
    m_viewfinder = new QLabel(this);
    m_viewfinder->setObjectName("viewfinder");
    m_viewfinder->setAlignment(Qt::AlignCenter);
    m_viewfinder->setText("Starting camera preview…");
    m_messageLabel = new QLabel("Starting camera…", this);
    m_messageLabel->setObjectName("cameraHint");

    auto *buttonRow = new QHBoxLayout;
    auto *cancelButton = new QPushButton("Cancel", this);
    m_captureButton = new QPushButton("Capture photo", this);
    m_captureButton->setObjectName("captureButton");
    m_captureButton->setEnabled(false);
    buttonRow->addStretch();
    buttonRow->addWidget(cancelButton);
    buttonRow->addWidget(m_captureButton);

    layout->addWidget(title);
    layout->addWidget(hint);
    layout->addWidget(m_viewfinder, 1);
    layout->addWidget(m_messageLabel);
    layout->addLayout(buttonRow);

    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_captureButton, &QPushButton::clicked, this, [this] { capturePhoto(); });

    if (QMediaDevices::videoInputs().isEmpty()) {
        m_messageLabel->setText("No camera was detected on this Mac.");
        return;
    }

    m_camera = new QCamera(QMediaDevices::defaultVideoInput(), this);
    m_captureSession = new QMediaCaptureSession(this);
    m_imageCapture = new QImageCapture(this);
    m_videoSink = new QVideoSink(this);
    m_captureSession->setCamera(m_camera);
    m_captureSession->setImageCapture(m_imageCapture);
    m_captureSession->setVideoSink(m_videoSink);

    connect(m_videoSink, &QVideoSink::videoFrameChanged, this,
            [this](const QVideoFrame &frame) {
                const QImage image = frame.toImage();
                if (image.isNull()) return;

                // A selfie preview should behave like a mirror. This affects
                // only the displayed frame, not the image saved by QImageCapture.
                const QPixmap mirrored = QPixmap::fromImage(image.flipped(Qt::Horizontal));
                m_viewfinder->setPixmap(mirrored.scaled(
                    m_viewfinder->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
                ));
            });

    connect(m_imageCapture, &QImageCapture::readyForCaptureChanged, this, [this](bool ready) {
        m_captureButton->setEnabled(ready);
        if (ready) m_messageLabel->setText("Camera ready. Frame your photo, then capture it.");
    });
    connect(m_imageCapture, &QImageCapture::imageSaved, this,
            [this](int, const QString &fileName) {
                m_capturedImagePath = fileName;
                accept();
            });
    connect(m_imageCapture, &QImageCapture::errorOccurred, this,
            [this](int, QImageCapture::Error, const QString &errorString) {
                m_messageLabel->setText("Camera error: " + errorString);
                m_captureButton->setEnabled(true);
            });

    m_camera->start();
}

CameraCaptureDialog::~CameraCaptureDialog() {
    if (m_camera) m_camera->stop();
}

QString CameraCaptureDialog::capturedImagePath() const {
    return m_capturedImagePath;
}

void CameraCaptureDialog::capturePhoto() {
    if (!m_imageCapture || !m_imageCapture->isReadyForCapture()) {
        m_messageLabel->setText("The camera is not ready yet. Please wait a moment.");
        return;
    }

    const QString captureDirectory = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
                                         .filePath("smart-photo-gallery-captures");
    QDir().mkpath(captureDirectory);
    const QString fileName = "capture_" +
        QDateTime::currentDateTimeUtc().toString("yyyyMMdd_hhmmss_zzz") + ".jpg";

    m_captureButton->setEnabled(false);
    m_messageLabel->setText("Saving photo…");
    m_imageCapture->captureToFile(QDir(captureDirectory).filePath(fileName));
}
