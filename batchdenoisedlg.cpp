#include "batchdenoisedlg.h"
#include "ui_batchdenoisedlg.h"

#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include "mainwindow.h"

#define ManWi ((MainWindow*)pMainWindow)

BatchDenoiseDlg::BatchDenoiseDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BatchDenoiseDlg)
{
    ui->setupUi(this);
    ProcessedFiles = 0;
    CurrentIndex = 0;
    Failures = 0;

}


// can't define in header because InferDetails belongs to mainwindow.h and including it in this dlg's .h would case circular dependency error
InferDetails MakeInferDetails(const std::vector<float>& InAudat,const QString& FilePath,unsigned InSampleRate,int32_t OutSampleRate)
{
    InferDetails Dets;
    Dets.F0 = 0.0f;
    Dets.Speed = 0.0f;
    Dets.Energy = 0.0f;
    Dets.pItem = nullptr; // the mainwindow's function will make an item for us.
    Dets.Prompt = "";
    Dets.SpeakerID = OutSampleRate; // SpeakerID will double as resample when a denoise only job is requested.
    Dets.EmotionID = -1;
    Dets.Denoise = true;
    Dets.Amplification = 1.f;
    Dets.ExportFileName = FilePath;


    Dets.VoiceName = "";
    Dets.ForcedAudio = InAudat;
    Dets.SampleRate = InSampleRate;

    return Dets;

}


BatchDenoiseDlg::~BatchDenoiseDlg()
{
    delete ui;
}

void BatchDenoiseDlg::IterateDo()
{

    if (ProcessedFiles == Files.size() && ManWi->GetCountItems() == 0)
    {
        // It's done!
        delete timIter;
        SetControls(true);

        return;

    }

    if (ManWi->GetCountItems() != 0)
        return;


    ManWi->DenDone = 0;
    if (CurrentIndex + ui->spbBatchSz->value() > Files.size())
        ManWi->DenBatchSize = Files.size() - CurrentIndex;

    for (int32_t i = 0;i < ui->spbBatchSz->value();i++)
    {



        QString CurrentFn = Files[CurrentIndex];

        AudioFile<float> AudFile;
        InferDetails CurrentDets;
        try {
            AudFile.load(CurrentFn.toStdString());

            CurrentDets = MakeInferDetails(AudFile.samples[0],CurrentFn,AudFile.getSampleRate(),ui->spbOutSR->value());

        }  catch (...) {

           CurrentIndex += 1; // NOT i !!!!!!!
           ProcessedFiles += 1;
           ++Failures;

           if (CurrentIndex > Files.size() - 1)
               break;

           continue;
        }

        ManWi->PushToInfers(CurrentDets);


        CurrentIndex += 1; // NOT i !!!!!!!
        ProcessedFiles += 1;

        if (CurrentIndex > Files.size() - 1)
            break;


    }
    SetLabel();








}

void BatchDenoiseDlg::on_btnFindFolder_clicked()
{

    QString Dir = QFileDialog::getExistingDirectory(this, tr("Find base folder of your WAVs"),
                                                "",
                                                QFileDialog::ShowDirsOnly
                                                | QFileDialog::DontResolveSymlinks);

    ui->edtFolPath->setText(Dir);

    UpdateDirectory();

}

void BatchDenoiseDlg::on_edtFolPath_editingFinished()
{
    UpdateDirectory();

}

void BatchDenoiseDlg::SetLabel()
{
    ui->lblFiles->setText(QString(QString::number(ProcessedFiles) + " / " + QString::number(Files.size()) + " files, " + QString::number(Failures) + " failures.") );

    ui->pgFiles->setValue(ProcessedFiles);
    ui->pgFiles->update();
}

void BatchDenoiseDlg::UpdateDirectory()
{
    if (ui->edtFolPath->text().isEmpty())
        return;

    if (Files.size())
        Files.clear();

    QDirIterator DirIt(ui->edtFolPath->text(),QDirIterator::Subdirectories);
    while (DirIt.hasNext())
    {
        DirIt.next();
        if (QFileInfo(DirIt.filePath()).isFile() && QFileInfo(DirIt.filePath()).suffix() == "wav")
            Files.push_back(DirIt.filePath());
    }
    CurrentIndex = 0;
    ProcessedFiles = 0;
    Failures = 0;

    ui->pgFiles->setRange(0,Files.size());


    SetLabel();


}

void BatchDenoiseDlg::on_btnStart_clicked()
{



    CurrentIndex = 0;
    ProcessedFiles = 0;
    Failures = 0;
    ManWi->DenBatchSize = ui->spbBatchSz->value();

    timIter = new QTimer(this);
    timIter->setSingleShot(false);
    timIter->setInterval(1000);

    connect(timIter,&QTimer::timeout,this,&BatchDenoiseDlg::IterateDo);

    timIter->start();

    SetControls(false);
}

void BatchDenoiseDlg::SetControls(bool En)
{
    ui->edtFolPath->setEnabled(En);
    ui->spbBatchSz->setEnabled(En);
    ui->btnStart->setEnabled(En);
    ui->btnFindFolder->setEnabled(En);

}
