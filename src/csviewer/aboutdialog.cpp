#include "aboutdialog.h"
#include "ui_about.h"
#include "./app_version.h"

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutWidget)
{
    ui->setupUi(this);

    ui->closeButton->setProperty("isCSStyle", true);

    QString appName = QString("<strong>%1 %2</strong>").arg(APP_NAME).arg(APP_VERSION);
    ui->appNameLabel->setText(appName);

    QString appBuiltTime = QString(tr("Built on %1")).arg(APP_BUILD_TIME);
    ui->builtInfo->setText(appBuiltTime);

    connect(ui->closeButton, &QPushButton::clicked, this, [=]()
        {
            close();
        });
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::onTranslate()
{
    ui->retranslateUi(this);
    QString appName = QString("<strong>%1 %2</strong>").arg(APP_NAME).arg(APP_VERSION);
    ui->appNameLabel->setText(appName);

    QString appBuiltTime = QString(tr("Built on %1")).arg(APP_BUILD_TIME);
    ui->builtInfo->setText(appBuiltTime);
}