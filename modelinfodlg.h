#ifndef MODELINFODLG_H
#define MODELINFODLG_H

#include <QDialog>

namespace Ui {
class ModelInfoDlg;
}

class ModelInfoDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ModelInfoDlg(QWidget *parent = nullptr);
    ~ModelInfoDlg();

    void SetInfo(const QString& ModelName,const QString& Info,int32_t InVersion,const QString& Author,const QString& Repo,const QString& MelGen,const QString& Vocoder);

private:
    Ui::ModelInfoDlg *ui;
};

#endif // MODELINFODLG_H
