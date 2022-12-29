#ifndef _CS_ABOUT_DIALOG_H
#define _CS_ABOUT_DIALOG_H

#include <QDialog>

namespace Ui
{
    class AboutWidget;
}

class AboutDialog : public QDialog
{
public:
    AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog();
    
    void onTranslate();
private:
    Ui::AboutWidget* ui;
};

#endif // _CS_ABOUT_DIALOG_H