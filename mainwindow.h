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
    void OnAudioRecv(std::vector<float> InDat);
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

private:
    bool CanPlayAudio;
    QStringList ListDirs(const QString& ParentDir);
    float RangeToFloat(int val);
    void PlayBuffer(QBuffer* pBuff);

    void AdvanceBuffer();

    bool MustExplicitlyIterateQueue();
    void PopulateComboBox();

    std::vector<QBuffer*> AudBuffs;

    size_t CurrentBuffIndex;

    QStringList SuperWordSplit(const QString& InStr,int MaxLen);


    std::queue<InferDetails> Infers;
    void IterateQueue();

    void DoInference(const InferDetails& Dets);

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
