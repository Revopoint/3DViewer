#ifndef _CS_CAMERA_PLAYER_DIALOG
#define _CS_CAMERA_PLAYER_DIALOG

#include <QDialog>

namespace Ui {
    class CameraPlayerWidget;
}

class CameraPlayerDialog : public QDialog
{
    Q_OBJECT
public:
    CameraPlayerDialog(QWidget* parent = nullptr);
    ~CameraPlayerDialog();
private:
    Ui::CameraPlayerWidget* ui;
};

#endif  // _CS_CAMERA_PLAYER_DIALOG