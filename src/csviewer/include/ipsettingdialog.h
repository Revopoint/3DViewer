#include <QDialog>

namespace Ui {
    class IpSettingWidget;
}

class IpSettingDialog : public QDialog
{
    Q_OBJECT
public:
    IpSettingDialog(QWidget* parent = nullptr);
    ~IpSettingDialog();
    void showEvent(QShowEvent* event) override;
    void onTranslate();
signals:
    void showMessage(QString msg, int timeout = 0);
private:
    void updateIpInfo();
private:
    void onApply();
private:
    Ui::IpSettingWidget* ui;
};