#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Voice.h"
#include <QStringList>
#include <QAudioFormat>
#include "voicemanager.h"
#include <QBuffer>
#include <QListWidgetItem>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    VoiceManager VoMan;
    QAudioFormat StdFmt;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void OnAudioRecv(std::vector<float> InDat);

private slots:
    void on_btnInfer_clicked();

    void on_btnLoad_clicked();

    void on_cbModels_currentIndexChanged(const QString &arg1);

    void on_sliEnergy_sliderMoved(int position);

    void on_sliSpeed_sliderMoved(int position);

    void on_lstUtts_itemDoubleClicked(QListWidgetItem *item);

    void on_btnClear_clicked();

    void on_btnExportSel_clicked();

private:
    QStringList ListDirs(const QString& ParentDir);
    float RangeToFloat(int val);
    void PlayBuffer(QBuffer* pBuff);

    void PopulateComboBox();

    std::vector<QBuffer*> AudBuffs;


    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
