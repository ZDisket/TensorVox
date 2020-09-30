#include "modelinfodlg.h"
#include "ui_modelinfodlg.h"

ModelInfoDlg::ModelInfoDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelInfoDlg)
{
    ui->setupUi(this);
}

ModelInfoDlg::~ModelInfoDlg()
{
    delete ui;
}

void ModelInfoDlg::SetInfo(const QString &ModelName, const QString &Info, int32_t InVersion, const QString &Author, const QString &Repo, const QString &MelGen, const QString &Vocoder, uint32_t SampleRate)
{
    ui->lblAuthor->setText("Author: " + Author);
    ui->lblVersion->setText("Version: " + QString::number(InVersion) + "  ");
    ui->redtModelInfo->setText(QString(Info).replace("(/NL)","\n"));

    ui->lblModelTitle->setText(ModelName);

    ui->lblModelArchitecture->setText("Architecture: " + Repo + " " + MelGen + " & " + Vocoder);
    ui->lblSampleRate->setText("Sampling rate: " + QString::number(SampleRate / 1000) + "KHz");
}
