#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Voice.h"
#include <QStringList>
#include <QAudioFormat>
#include "voicemanager.h"
#include <QBuffer>
#include <QListWidgetItem>
#include <QAudio>
#include <QAudioOutput>
#include <queue>

#include "phonetichighlighter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct InferDetails{
  QString Prompt;
  float Energy;
  float Speed;
  float F0;
  QListWidgetItem* pItem;
  int32_t SpeakerID;
  QString VoiceName;



};

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    VoiceManager VoMan;
    QAudioFormat StdFmt;
    QAudioOutput* StdOutput;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void OnAudioRecv(std::vector<float> InDat,std::chrono::duration<double> infer_span);
    void OnAudioStateChange(QAudio::State newState);

private slots:
    void on_btnInfer_clicked();

    void on_btnLoad_clicked();

    void on_cbModels_currentIndexChanged(const QString &arg1);

    void on_sliEnergy_sliderMoved(int position);

    void on_sliSpeed_sliderMoved(int position);

    void on_lstUtts_itemDoubleClicked(QListWidgetItem *item);

    void on_btnClear_clicked();

    void on_btnExportSel_clicked();

    void on_actionExport_performance_report_triggered();

    void on_chkRecPerfLog_clicked(bool checked);

    void on_btnExReport_clicked();

    void on_btnRefreshList_clicked();

    void on_sliF0_sliderMoved(int position);

private:
    bool CanPlayAudio;
    QStringList ListDirs(const QString& ParentDir);
    float RangeToFloat(int val);
    void PlayBuffer(QBuffer* pBuff);
    bool RecPerfLines;

    void AdvanceBuffer();

    bool MustExplicitlyIterateQueue();
    void PopulateComboBox();

    std::vector<QBuffer*> AudBuffs;

    size_t CurrentBuffIndex;

    QStringList SuperWordSplit(const QString& InStr,int MaxLen);

    void ProcessCurlies(QString& ModTxt);


    std::queue<InferDetails> Infers;

    QStringList PerfReportLines;
    void IterateQueue();

    void DoInference(InferDetails &Dets);

    PhoneticHighlighter* pHigh;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
