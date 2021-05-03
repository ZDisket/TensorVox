#ifndef BATCHDENOISEDLG_H
#define BATCHDENOISEDLG_H

#include <QDialog>
#include <QTimer>
namespace Ui {
class BatchDenoiseDlg;
}

class BatchDenoiseDlg : public QDialog
{
    Q_OBJECT

public:
    explicit BatchDenoiseDlg(QWidget *parent = nullptr);
    ~BatchDenoiseDlg();



    // if we included mainwindow.h in here it would result in circular dependency problem so we include it in the .cpp
    // and make it a void* here
    void* pMainWindow;

private slots:


    void IterateDo();
    void on_btnFindFolder_clicked();

    void on_edtFolPath_editingFinished();

    void on_btnStart_clicked();

private:

    void SetControls(bool En);

    QStringList Files;
    QTimer* timIter;
    int32_t ProcessedFiles;
    int32_t CurrentIndex;
    int32_t Failures;



    void SetLabel();
    void UpdateDirectory();
    Ui::BatchDenoiseDlg *ui;
};

#endif // BATCHDENOISEDLG_H
