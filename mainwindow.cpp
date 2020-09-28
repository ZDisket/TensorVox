#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QDirIterator>
#include <QSplitter>

#include <QFileDialog>

#include "voxer.h"

#include "ext/ByteArr.h"
#include "ext/ZFile.h"

#include "phddialog.h"
#include "framelesswindow.h"
#include <QMessageBox>
#include "modelinfodlg.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    PopulateComboBox();
    qRegisterMetaType< std::vector<float> >( "std::vector<float>" );

    qRegisterMetaType< QVector<int> >( "QVector<int>" );
    qRegisterMetaType<std::chrono::duration<double>>("std::chrono::duration<double>");

    ui->splitter->setSizes(QList<int>() << width() * 0.8  << width() * 0.2 );

    StdFmt.setSampleSize(sizeof(float) * 8);
    StdFmt.setSampleType(QAudioFormat::Float);

    // This is already set in the constructor, but just to be extra, extra sure...
    StdFmt.setByteOrder(QAudioFormat::Endian(QSysInfo::ByteOrder));

    StdFmt.setSampleRate(CommonSampleRate);
    StdFmt.setChannelCount(1);
    StdFmt.setCodec("audio/pcm");

    CanPlayAudio = true;

    StdOutput = new QAudioOutput(StdFmt,this);

    CurrentBuffIndex = 0;

     connect(StdOutput,&QAudioOutput::stateChanged, this, &MainWindow::OnAudioStateChange);

    RecPerfLines = false;

    pHigh = new PhoneticHighlighter(ui->edtInput->document());

    ui->cbSpeaker->setVisible(false);
    ui->lblSpeaker->setVisible(false);

    ui->cbEmotions->setVisible(false);
    ui->lblEmotions->setVisible(false);





    CurrentInferIndex = 0;

    PhonDict.Import(QCoreApplication::applicationDirPath() + "/dict.phd");
    pTaskBtn = new QWinTaskbarButton(this);
    pTskProg = pTaskBtn->progress();

    SetDict();




}

void MainWindow::showEvent(QShowEvent *e)
{

#ifdef Q_OS_WIN
    pTaskBtn->setWindow(windowHandle());
    pTaskBtn->setOverlayIcon(QIcon(":/res/stdico.png"));
    pTaskBtn->progress()->show();

#endif

    e->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnAudioRecv(std::vector<float> InDat, std::chrono::duration<double> infer_span)
{

    IterateQueue();


    for (float& f : InDat)
          f *= (float)ui->sliVolBoost->value() / 1000.f;


    QBuffer* Buf = new QBuffer(this);
    Buf->setData((const char*)InDat.data(),sizeof(float) * InDat.size());


    AudBuffs.push_back(Buf);
    if (CanPlayAudio)
        PlayBuffer(Buf);


    if (RecPerfLines){

        double InferSecs = (double)InDat.size() / CommonSampleRate;
        double InferTime = infer_span.count();

        // https://enacademic.com/dic.nsf/enwiki/3796485
        double RealTimeFactor = InferTime / InferSecs;

        QString Pline = "Inferred " + QString::number(InferSecs,'f',3) + " seconds of audio in " + QString::number(InferTime,'f',3) + " seconds; " + "RTF: " + QString::number(RealTimeFactor,'f',3);
        PerfReportLines.push_back(Pline);

    }

    bool NoInfers = Infers.empty();

    ui->btnExportSel->setEnabled(NoInfers);
    ui->btnExReport->setEnabled(NoInfers);






}

void MainWindow::OnAudioStateChange(QAudio::State newState)
{
    if (newState == QAudio::IdleState || newState == QAudio::StoppedState)
    {

        // If the user queues up multiple utterances then due to the fast inference speed we can't play them all at once
        // It then becomes necessary that we advance on the event that audio finishes playing.
        CurrentBuffIndex += 1;
        if (CurrentBuffIndex < AudBuffs.size())
            AdvanceBuffer();
        else
            CanPlayAudio = true;


    }
}


void MainWindow::on_btnInfer_clicked()
{
    // While the Voice class can already autoload, we explicitly do so here so we can handle everything (note text, disabling)

    if (VoMan.FindVoice(ui->cbModels->currentText(),false) == -1)
        on_btnLoad_clicked();




    QString RawInput = ui->edtInput->toPlainText();
    QString Input = RawInput.replace("\n"," ");
    const int MaxShowInputLen = ui->lstUtts->size().width() / 6;








    QStringList InputSplits;

    if (ui->rbSplitWord->isChecked())
        InputSplits = SuperWordSplit(Input,ui->spbSeqLen->value());
    else
        InputSplits = ui->edtInput->toPlainText().split(QRegExp("[\r\n]"),QString::SkipEmptyParts);


    for (QString& idvInput : InputSplits)
    {
        if (idvInput.isEmpty())
            continue;

        InferDetails Dets;
        ProcessCurlies(idvInput);

        QString InputForShow = idvInput;

        if (idvInput.length() > MaxShowInputLen)
            InputForShow = idvInput.mid(0,MaxShowInputLen) + "(...)";



        QListWidgetItem* widItm = new QListWidgetItem(InputForShow,ui->lstUtts);


        if (ui->chkBiPad->isChecked())
            idvInput = "@SIL " + idvInput;

        Dets.F0 = RangeToFloat(ui->sliF0->value());
        Dets.Speed = RangeToFloat(ui->sliSpeed->value());
        Dets.Energy = RangeToFloat(ui->sliEnergy->value());
        Dets.pItem = widItm;
        Dets.Prompt = idvInput + " @SIL @END";
        Dets.SpeakerID = 0;
        Dets.EmotionID = -1;

        if (ui->cbSpeaker->isVisible())
            Dets.SpeakerID = ui->cbSpeaker->currentIndex();

        if (ui->cbEmotions->isVisible())
            Dets.EmotionID = ui->cbEmotions->currentIndex();

        Dets.VoiceName = ui->cbModels->currentText();

        Infers.push(Dets);

    }


    pTskProg->show();

    // If this is the first iteration (indicated by an empty list), or all are done (no list widget items are in process color),
    // we explicitly iterate it. Otherwise, we let the active triggers (after audio data is received) iterate it for us.
    // after adding it.
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

void MainWindow::PlayBuffer(QBuffer *pBuff,bool ByUser)
{

    if (!ui->chkAutoPlay->isChecked() && !ByUser)
        return;

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
        if (CurrentStr.size() > 0)
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

void MainWindow::ProcessCurlies(QString &ModTxt)
{
    QString MatchExp = "\\{(\\s*?.*?)*?\\}";
    QRegularExpression& PhonemeExp = pHigh->PhonemeExp;
    QRegularExpressionMatchIterator MatchIter = PhonemeExp.globalMatch(ModTxt);

    while (MatchIter.hasNext()) {
        QRegularExpressionMatch match = MatchIter.next();
        QString ToProcess = ModTxt.mid(match.capturedStart(),match.capturedLength());
        QStringList Toks = ToProcess.split(" ",QString::SplitBehavior::SkipEmptyParts,Qt::CaseInsensitive);

        QStringList NewTokens;
        for (QString& Tk : Toks){
            Tk = Tk.replace("{","").replace("}","");
            if (Tk.isEmpty())
                continue;


            NewTokens.push_back("@" + Tk);

        }

        QString Assembled = NewTokens.join(" ");
        QString BeforeTxt = ModTxt.mid(0,match.capturedStart());
        QString AfterTxt = ModTxt.mid(match.capturedEnd());

        ModTxt = BeforeTxt + Assembled + AfterTxt;
        // After every process we take the string and rebuild it, which makes the old indices outdated and dangerous to use.
        // We match again.
        MatchIter = PhonemeExp.globalMatch(ModTxt);
    }

}


void MainWindow::IterateQueue()
{
    if (!Infers.size())
        return;

   ++CurrentInferIndex;

    pTskProg->setRange(0,ui->lstUtts->count());
   pTskProg->setValue(CurrentInferIndex);

    DoInference(Infers.front());
    Infers.pop();

}

void MainWindow::DoInference(InferDetails &Dets)
{

    Voxer* VoxThread = new Voxer;
    VoxThread->F0 = Dets.F0;
    VoxThread->Energy = Dets.Energy;
    VoxThread->Speed = Dets.Speed;
    VoxThread->Prompt = Dets.Prompt;
    VoxThread->pAttItem = Dets.pItem;
    VoxThread->SpeakerID = Dets.SpeakerID;
    size_t VoiceID = (size_t)VoMan.FindVoice(Dets.VoiceName,true);
    //Auto-load is true, so we will always get a good pointer.
    VoxThread->pAttVoice = VoMan[VoiceID];
    VoxThread->SampleRate = VoMan[VoiceID]->GetInfo().SampleRate;
    VoxThread->EmotionID = Dets.EmotionID;

    connect(VoxThread,&Voxer::Done,this,&MainWindow::OnAudioRecv);

    VoxThread->start();

}



void MainWindow::on_btnLoad_clicked()
{
    size_t VoID = VoMan.LoadVoice(ui->cbModels->currentText());
    ui->btnLoad->setEnabled(false);
    std::string VoNote = VoMan[VoID]->GetInfo().Note;

    ui->lblModelNote->setText(QString::fromStdString(VoNote));
    HandleIsMultiSpeaker(VoID);

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


    PlayBuffer(AudBuffs[ui->lstUtts->row(item)],true);

}

void MainWindow::on_btnClear_clicked()
{
    ui->lstUtts->clear();
    AudBuffs.clear();
    CurrentBuffIndex = 0;
    PerfReportLines.clear();
    CurrentInferIndex = 0;

    pTskProg->hide();

}

void MainWindow::on_btnExportSel_clicked()
{
    if (ui->lstUtts->selectedItems().size() == 0){
        QMessageBox::critical(this,"Error","You have to select an item to export selection.");
        return;

    }


    QString ofname = QFileDialog::getSaveFileName(this, tr("Export WAV file"), "Utt", tr("WAV, float32 PCM (*.wav)"));
    if (!ofname.size())
        return;

    std::vector<float> Audat;
    QByteArray& CurrentBuff = AudBuffs[(size_t)ui->lstUtts->currentRow()]->buffer();

    Audat.resize((size_t)CurrentBuff.size() / sizeof(float));


    smemcpy(Audat.data(),Audat.size() * sizeof(float),CurrentBuff.data(),(size_t)CurrentBuff.size());


    VoxUtil::ExportWAV(ofname.toStdString(),Audat,CommonSampleRate);

}

void MainWindow::on_actionExport_performance_report_triggered()
{
    QString ofname = QFileDialog::getSaveFileName(this, tr("Export performance report TXT file"), "log", tr("Text file (*.txt)"));
    if (!ofname.size())
        return;

    ZFile zfOut;
    zfOut.Open(ofname.toStdString(),EZFOpenMode::BinaryWrite);
    for (const QString& PerfLi : PerfReportLines)
        zfOut.WriteLine(PerfLi.toStdString());

    zfOut.Close();




}

void MainWindow::on_chkRecPerfLog_clicked(bool checked)
{
    RecPerfLines = checked;

}

void MainWindow::on_btnExReport_clicked()
{
    QString ofname = QFileDialog::getSaveFileName(this, tr("Export WAV file"), "Utt", tr("WAV, float32 PCM (*.wav)"));
    if (!ofname.size())
        return;

    std::vector<float> Audat;
    QByteArray CurrentBuff;

    for (const auto& IBuff : AudBuffs){
        CurrentBuff.append(IBuff->buffer());

    }

    Audat.resize((size_t)CurrentBuff.size() / sizeof(float));


    smemcpy(Audat.data(),Audat.size() * sizeof(float),CurrentBuff.data(),(size_t)CurrentBuff.size());


    VoxUtil::ExportWAV(ofname.toStdString(),Audat,CommonSampleRate);


}

void MainWindow::on_btnRefreshList_clicked()
{
    PopulateComboBox();

}

void MainWindow::on_sliF0_sliderMoved(int position)
{
    ui->lblF0Show->setText(QString::number(position) + "%");


}

void MainWindow::on_cbModels_currentTextChanged(const QString &arg1)
{
    int32_t CurrentIndex = VoMan.FindVoice(arg1,false);

    if (CurrentIndex != -1){
        ui->lblModelNote->setText(QString::fromStdString(VoMan[(size_t)CurrentIndex]->GetInfo().Note));
        HandleIsMultiSpeaker((size_t)CurrentIndex);

    }

}

void MainWindow::on_hkInfer_triggered()
{
    on_btnInfer_clicked();
}

void MainWindow::HandleIsMultiSpeaker(size_t inVid)
{
    HandleIsMultiEmotion(inVid);

    Voice& CurrentVoice = *VoMan[inVid];

    if (!CurrentVoice.GetSpeakers().size()){
        ui->lblSpeaker->setVisible(false);
        ui->cbSpeaker->setVisible(false);

        return;
    }


    ui->cbSpeaker->clear();

    for (const auto& SpkName : CurrentVoice.GetSpeakers())
    {
        ui->cbSpeaker->addItem(QString::fromStdString(SpkName));


    }

    ui->lblSpeaker->setVisible(true);
    ui->cbSpeaker->setVisible(true);


    if (!CurrentVoice.GetEmotions().size()){
        ui->lblEmotions->setVisible(false);
        ui->cbEmotions->setVisible(false);
        return;
    }




}

void MainWindow::HandleIsMultiEmotion(size_t inVid)
{
    Voice& CurrentVoice = *VoMan[inVid];

    if (!CurrentVoice.GetEmotions().size()){
        ui->lblEmotions->setVisible(false);
        ui->cbEmotions->setVisible(false);
        return;
    }


    ui->cbEmotions->clear();

    for (const auto& EmName : CurrentVoice.GetEmotions())
    {
        ui->cbEmotions->addItem(QString::fromStdString(EmName));


    }

    ui->lblEmotions->setVisible(true);
    ui->cbEmotions->setVisible(true);




}

void MainWindow::on_actionOverrides_triggered()
{
    FramelessWindow FDlg(this);
    FDlg.setWindowIcon(QIcon(":/res/phoneticdico.png"));
    FDlg.setWindowTitle("Phonetic Overrides");
    FDlg.SetTitleBarBtns(false,false,true);
    FDlg.resize(640,480);

    PhdDialog Dlg(this);
    Dlg.Entrs = PhonDict.Entries;

    FDlg.setContent(&Dlg);
    FDlg.ContentDlg(&Dlg);

    FDlg.show();

    Dlg.setModal(true);

    int code = Dlg.exec();
    if (code == QDialog::Accepted)
    {
        PhonDict.Entries = Dlg.Entrs;
        PhonDict.Export(QCoreApplication::applicationDirPath() + "/dict.phd");
        SetDict();



    }

}

void MainWindow::SetDict()
{
    VoMan.SetDict(PhonDict.Entries);
    for (Voice*& Vo : VoMan.GetVoices())
    {
        Vo->SetDictEntries(PhonDict.Entries);

    }

}

void MainWindow::on_actionRefresh_model_listing_triggered()
{
    PopulateComboBox();
}

void MainWindow::on_btnLoadInfo_clicked()
{
    int32_t CurrentIndex = VoMan.FindVoice(ui->cbModels->currentText(),false);

    if (CurrentIndex == -1){
        QMessageBox::critical(this,"Error","You have to load the model before accessing its info");
        return;

    }
    VoiceInfo Voi = VoMan[(size_t)CurrentIndex]->GetInfo();

    FramelessWindow FDlg(this);
    FDlg.setWindowIcon(QIcon(":/res/infico.png"));
    FDlg.setWindowTitle("Model Info");
    FDlg.SetTitleBarBtns(false,false,true);
    FDlg.resize(500,450);

    ModelInfoDlg Dlg(this);

    FDlg.setContent(&Dlg);
    FDlg.ContentDlg(&Dlg);

    QString MdlDesc = QString::fromStdString(Voi.Description);
    std::string MiExpanded = VoMan[(size_t)CurrentIndex]->GetModelInfo();

    if (MiExpanded.size() > 1)
        MdlDesc = QString::fromStdString(MiExpanded);


    FDlg.show();
    Dlg.SetInfo(QString::fromStdString(Voi.Name),MdlDesc,Voi.Version,QString::fromStdString(Voi.Author),
                QString::fromStdString(Voi.Architecture.s_Repo),QString::fromStdString(Voi.Architecture.s_Text2Mel),
                QString::fromStdString(Voi.Architecture.s_Vocoder));


    Dlg.setModal(true);

    Dlg.exec();

}
