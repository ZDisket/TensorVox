#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QDirIterator>
#include <QSplitter>

#include <QFileDialog>

#include "voxer.h"

#include "ext/ByteArr.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    PopulateComboBox();
    qRegisterMetaType< std::vector<float> >( "std::vector<float>" );

    qRegisterMetaType< QVector<int> >( "QVector<int>" );

    ui->splitter->setSizes(QList<int>() << width() * 0.8  << width() * 0.2 );

    StdFmt.setSampleSize(sizeof(float) * 8);
    StdFmt.setSampleType(QAudioFormat::Float);

    // This is already set in the constructor, but just to be extra, extra sure...
    StdFmt.setByteOrder(QAudioFormat::Endian(QSysInfo::ByteOrder));

    StdFmt.setSampleRate(22050);
    StdFmt.setChannelCount(1);
    StdFmt.setCodec("audio/pcm");

    CanPlayAudio = true;

    StdOutput = new QAudioOutput(StdFmt,this);

    CurrentBuffIndex = 0;

     connect(StdOutput,&QAudioOutput::stateChanged, this, &MainWindow::OnAudioStateChange);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnAudioRecv(std::vector<float> InDat)
{

    IterateQueue();
    QBuffer* Buf = new QBuffer(this);
    Buf->setData((const char*)InDat.data(),sizeof(float) * InDat.size());


    AudBuffs.push_back(Buf);
    if (CanPlayAudio)
        PlayBuffer(Buf);

}

void MainWindow::OnAudioStateChange(QAudio::State newState)
{
    if (newState == QAudio::IdleState || newState == QAudio::StoppedState)
    {
        CanPlayAudio = true;

        // If the user queues up multiple utterances then due to the fast inference speed we can't play them all at once
        // It then becomes necessary that we advance on the event that audio finishes playing.
        CurrentBuffIndex += 1;
        if (CurrentBuffIndex < AudBuffs.size())
            AdvanceBuffer();


    }
}


void MainWindow::on_btnInfer_clicked()
{
    // If this is the first iteration (indicated by an empty list), or all are done (no list widget items are in process color),
    // we explicitly iterate it. Otherwise, we let the active triggers (after audio data is received) iterate it for us.
    // after adding it.
    bool IterQNow = MustExplicitlyIterateQueue();


    QString RawInput = ui->edtInput->toPlainText();
    QString Input = RawInput.replace("\n","");
    const int MaxShowInputLen = ui->lstUtts->size().width() / 4;








    QStringList InputSplits;

    if (ui->rbSplitWord->isChecked())
        InputSplits = SuperWordSplit(Input,ui->spbSeqLen->value());
    else
        InputSplits = RawInput.split("\n",QString::SplitBehavior::SkipEmptyParts,Qt::CaseInsensitive);


    for (const QString& idvInput : InputSplits)
    {
        InferDetails Dets;
        QString InputForShow = idvInput;

        if (Input.length() > MaxShowInputLen)
            InputForShow = idvInput.mid(0,MaxShowInputLen) + "(...)";


        QListWidgetItem* widItm = new QListWidgetItem(InputForShow,ui->lstUtts);



        Dets.F0 = 1.f;
        Dets.Speed = RangeToFloat(ui->sliSpeed->value());
        Dets.Energy = RangeToFloat(ui->sliEnergy->value());
        Dets.pItem = widItm;
        Dets.Prompt = idvInput;
        Dets.SpeakerID = 0;
        Dets.VoiceName = ui->cbModels->currentText();

        Infers.push(Dets);

    }



    if (MustExplicitlyIterateQueue())
        IterateQueue();







}

QStringList MainWindow::ListDirs(const QString &ParentDir)
{
    QStringList all_dirs;
    QDirIterator directories(ParentDir, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);

    while(directories.hasNext()){
        directories.next();
        all_dirs << directories.filePath().replace("models/","");
    }

    return all_dirs;
}

float MainWindow::RangeToFloat(int val)
{
    int procval = 200 - val;
    if (procval == 0)
        procval = 1;

    return (float)procval / 100.f;

}

void MainWindow::PlayBuffer(QBuffer *pBuff)
{

    pBuff->open(QBuffer::ReadWrite);

    StdOutput->start(pBuff);
    CanPlayAudio = false;


}

void MainWindow::AdvanceBuffer()
{
    PlayBuffer(AudBuffs[CurrentBuffIndex]);

}

bool MainWindow::MustExplicitlyIterateQueue()
{
    if (!ui->lstUtts->count())
        return true;

    for (int i = 0; i < ui->lstUtts->count();i++)
    {
        if (ui->lstUtts->item(i)->backgroundColor() == InProcessColor)
            return false;

    }


    return true;

}

void MainWindow::PopulateComboBox()
{

    ui->cbModels->clear();
    ui->cbModels->insertItems(0,ListDirs("models"));


}

QStringList MainWindow::SuperWordSplit(const QString &InStr, int MaxLen)
{
    QStringList RawWords = InStr.split(" ",QString::SplitBehavior::SkipEmptyParts,Qt::CaseInsensitive);
    int AmtWords = RawWords.size();

    int Idx = 0;
    QString CurrentStr = "";

    QStringList SplitStrs;

    while (Idx < AmtWords)
    {
        if (CurrentStr.size() > 1)
            CurrentStr.append(" ");

        CurrentStr.append(RawWords[Idx]);

        if (CurrentStr.length() > MaxLen){
            SplitStrs.append(CurrentStr);
            CurrentStr = "";

        }


        Idx += 1;

        // Add the last string
        if (Idx == AmtWords)
            SplitStrs.append(CurrentStr);






    }

    return SplitStrs;

}

void MainWindow::IterateQueue()
{
    if (!Infers.size())
        return;
    DoInference(Infers.front());
    Infers.pop();

}

void MainWindow::DoInference(const InferDetails &Dets)
{

    Voxer* VoxThread = new Voxer;
    VoxThread->F0 = Dets.F0;
    VoxThread->Energy = Dets.Energy;
    VoxThread->Speed = Dets.Speed;
    VoxThread->Prompt = Dets.Prompt;
    VoxThread->pAttItem = Dets.pItem;
    VoxThread->SpeakerID = 0;
    size_t VoiceID = (size_t)VoMan.FindVoice(Dets.VoiceName,true);
    //Auto-load is true, so we will always get a good pointer.
    VoxThread->pAttVoice = VoMan[VoiceID];
    VoxThread->SampleRate = VoMan[VoiceID]->GetInfo().SampleRate;

    connect(VoxThread,&Voxer::Done,this,&MainWindow::OnAudioRecv);

    VoxThread->start();

}



void MainWindow::on_btnLoad_clicked()
{
    size_t VoID = VoMan.LoadVoice(ui->cbModels->currentText());
    ui->btnLoad->setEnabled(false);
    std::string VoNote = VoMan[VoID]->GetInfo().Note;

    ui->lblModelNote->setText(QString::fromStdString(VoNote));

}

void MainWindow::on_cbModels_currentIndexChanged(const QString &arg1)
{

    ui->btnLoad->setEnabled(VoMan.FindVoice(arg1,false) == -1);


}

void MainWindow::on_sliEnergy_sliderMoved(int position)
{
    ui->lblEnergyShow->setText(QString::number(position) + "%");

}

void MainWindow::on_sliSpeed_sliderMoved(int position)
{
    ui->lblSpeedShow->setText(QString::number(position) + "%");
}

void MainWindow::on_lstUtts_itemDoubleClicked(QListWidgetItem *item)
{

    PlayBuffer(AudBuffs[ui->lstUtts->row(item)]);

}

void MainWindow::on_btnClear_clicked()
{
    ui->lstUtts->clear();
    AudBuffs.clear();
    CurrentBuffIndex = 0;

}

void MainWindow::on_btnExportSel_clicked()
{
    QString ofname = QFileDialog::getSaveFileName(this, tr("Export WAV file"), "Utt", tr("WAV, float32 PCM (*.wav)"));
    if (!ofname.size())
        return;

    std::vector<float> Audat;
    QByteArray& CurrentBuff = AudBuffs[(size_t)ui->lstUtts->currentRow()]->buffer();

    Audat.resize((size_t)CurrentBuff.size() / sizeof(float));


    smemcpy(Audat.data(),Audat.size() * sizeof(float),CurrentBuff.data(),(size_t)CurrentBuff.size());


    VoxUtil::ExportWAV(ofname.toStdString(),Audat,22050);

}
