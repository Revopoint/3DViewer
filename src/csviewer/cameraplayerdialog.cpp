#include "cameraplayerdialog.h"
#include "ui_cameraplayer.h"

CameraPlayerDialog::CameraPlayerDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CameraPlayerWidget)
{
    ui->setupUi(this);

    QVector<int> windows;
    windows.push_back(CAMERA_DATA_DEPTH);
    windows.push_back(CAMERA_DATA_RGB);

    ui->playerRenderWindow->onRenderWindowsUpdated(windows);
}

CameraPlayerDialog::~CameraPlayerDialog()
{
    delete ui;
}