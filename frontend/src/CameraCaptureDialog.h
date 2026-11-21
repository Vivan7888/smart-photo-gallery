#ifndef CAMERA_CAPTURE_DIALOG_H
#define CAMERA_CAPTURE_DIALOG_H

#include <QDialog>

class QCamera;
class QImageCapture;
class QMediaCaptureSession;
class QLabel;
class QPushButton;
class QVideoSink;

class CameraCaptureDialog : public QDialog {
public:
    explicit CameraCaptureDialog(QWidget *parent = nullptr);
    ~CameraCaptureDialog() override;

    QString capturedImagePath() const;

private:
    void capturePhoto();

    QCamera *m_camera = nullptr;
    QMediaCaptureSession *m_captureSession = nullptr;
    QImageCapture *m_imageCapture = nullptr;
    QVideoSink *m_videoSink = nullptr;
    QLabel *m_viewfinder = nullptr;
    QLabel *m_messageLabel = nullptr;
    QPushButton *m_captureButton = nullptr;
    QString m_capturedImagePath;
};

#endif
