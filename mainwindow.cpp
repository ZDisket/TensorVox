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
#include <LogitechLEDLib.h>
#include "track.h"
#define FwParent ((FramelessWindow*)pDarkFw)
#include <psapi.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    DoingBatchDenoising = false;

    ui->setupUi(this);
    PopulateComboBox();

    DoUpdateSplitAuto = true;
    qRegisterMetaType< std::vector<float> >( "std::vector<float>" );

    qRegisterMetaType< QVector<int> >( "QVector<int>" );
    qRegisterMetaType<std::chrono::duration<double>>("std::chrono::duration<double>");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType< TFTensor<float> >( "TFTensor<float>" );

    StatusLbl = new QLabel(ui->statusbar);
    ui->statusbar->addPermanentWidget(StatusLbl);



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

    ui->spbThreads->setValue(QThread::idealThreadCount());

    NumDone = 0;
    CurrentAmtThreads = 0;

    LogiLedAvailable = LogiLedInitWithName("Vox");
    if (LogiLedAvailable)
        LogiLedSetTargetDevice(LOGI_DEVICETYPE_RGB);

    UpdateLogiLed();

    ClipBrd = QGuiApplication::clipboard();
    connect(ClipBrd,&QClipboard::dataChanged,this,&MainWindow::OnClipboardDataChanged);


    QTimer* timMemUp = new QTimer(this);
    timMemUp->setSingleShot(false);
    timMemUp->setInterval(5000);


    connect(timMemUp,&QTimer::timeout,this,&MainWindow::OnMemoryUpdate);
    LastInferBatchSize = 0;

    timMemUp->start();
    //ui->widAudioPlot->hide();

    QCPColorGradient Viridis;
    // Build Viridis color gradient.
    Viridis.clearColorStops();
    Viridis.setColorStops({{0.0,QColor(54, 1, 81)},
                           {0.25,QColor(45, 62, 120)},
                           {0.5,QColor(30, 131, 121)},
                           {0.75,QColor(94, 200, 71)},
                           {1.0,QColor(245, 228 ,27)}
                          });

    QCPColorMap* ColMap = new QCPColorMap(ui->widAttention->xAxis, ui->widAttention->yAxis);
    ColMap->setName("Alignment");
    ui->widAttention->Map = ColMap;
    ColMap->setGradient(Viridis);

    ui->tabMetrics->hide();
    ui->tabMetrics->setTabEnabled(2,false);



    // The spectrogram in Python is shown with np.rot90, otherwise it looks wrong
    // So I flip the axes to make it look right, instead of bothering to do the math (I lost an evening trying this)
    QCPColorMap* SpecMap = new QCPColorMap(ui->widSpec->yAxis, ui->widSpec->xAxis);
    SpecMap->setName("Spectrogram");
    ui->widSpec->Map = SpecMap;
    SpecMap->setGradient(Viridis);

    DenBatchSize = 0;
    DenDone = 0;
    LastExportDir = QCoreApplication::applicationDirPath() + "/Utt.wav";





}

InferIDTrueID* MainWindow::FindByFirst(uint32_t inGetID)
{
    for (auto& ID : IdVec)
    {
        if (inGetID == ID.first)
            return &ID;

    }
    return nullptr;


}

void MainWindow::PushToInfers(InferDetails &InDets)
{

    if (!InDets.pItem)
        InDets.pItem = new QListWidgetItem(InDets.ExportFileName.split('/').last(),ui->lstUtts);




    Infers.push(InDets);
    pTskProg->show();

    IterateQueue();


}

int32_t MainWindow::GetCountItems()
{
    return ui->lstUtts->count();
}



void MainWindow::showEvent(QShowEvent *e)
{

#ifdef Q_OS_WIN
    pTaskBtn->setWindow(((FramelessWindow*)pDarkFw)->windowHandle());
    pTaskBtn->progress()->show();



#endif
    //FwParent->setWindowTitle("TensorVox");

   e->accept();
}

MainWindow::~MainWindow()
{
    on_btnClear_clicked();
    delete ui;
}

void MainWindow::OnAudioRecv(std::vector<float> InDat, TFTensor<float> InMel, std::chrono::duration<double> infer_span, uint32_t inID)
{
    if (inID == UINT32_MAX)
    {
        DoingBatchDenoising = true;
        ++DenDone;

        --CurrentAmtThreads;


        ui->chkMultiThreaded->setChecked(ui->lstUtts->count() > ui->spbThreads->value());





        if (DenDone == DenBatchSize){
            on_btnClear_clicked();
            return;

        }



        IterateQueue();

        return;

    }
    DoingBatchDenoising = false;



    QBuffer* Buf = new QBuffer(this);
    Buf->setData((const char*)InDat.data(),sizeof(float) * InDat.size());


    // We push to the mel and audbuff always at the same time, so the IDs will be the same.
    Mels.push_back(InMel);
    AudBuffs.push_back(Buf);

    IdVec.push_back(InferIDTrueID{inID,AudBuffs.size() - 1,-1});
    if (AllowedToPlayAudio())
    {
        PlayBuffer(Buf,false,inID);

    }


    if (RecPerfLines){

        double InferSecs = (double)InDat.size() / CommonSampleRate;
        double InferTime = infer_span.count();

        // https://enacademic.com/dic.nsf/enwiki/3796485
        double RealTimeFactor = InferTime / InferSecs;

        QString Pline = "Inferred " + QString::number(InferSecs,'f',3) + " seconds of audio in " + QString::number(InferTime,'f',3) + " seconds; " + "RTF: " + QString::number(RealTimeFactor,'f',3);
        PerfReportLines.push_back(Pline);

    }



    NumDone += 1;


    pTskProg->setRange(0,ui->lstUtts->count());
    pTskProg->setValue(NumDone);


    bool NoInfers = NumDone == ui->lstUtts->count();

    ui->btnExportSel->setEnabled(NoInfers);
    ui->btnExReport->setEnabled(NoInfers);

    if (NoInfers)
    {
        LogiLedSetLighting(0,100,50);


        // Prevent lighting from being flashed if it's just one or a few utterances.
        if (LastInferBatchSize > 3)
            LogiLedFlashLighting(0,100,50,6000,500);




    }
    else
    {
        float NDonef = (float)NumDone;
        float Cnt = (float)ui->lstUtts->count();

        float BlueLevel = NDonef / Cnt;
        BlueLevel *= 100.f;

        LogiLedSetLighting(20,0,(int)BlueLevel);


    }




    --CurrentAmtThreads;

    IterateQueue();
    CountBlues();



}

void MainWindow::OnAudioStateChange(QAudio::State newState)
{

    if (newState == QAudio::IdleState || newState == QAudio::StoppedState)
    {
        InferIDTrueID* FoundItm = FindBySecond(CurrentBuffIndex);

        if (!FoundItm)
            return;

        if ((int32_t)FoundItm->first > ui->lstUtts->count() - 1)
            return;

        ui->lstUtts->item(FoundItm->first)->setBackgroundColor(DoneColor);


        // If the user queues up multiple utterances then due to the fast inference speed we can't play them all at once
        // It then becomes necessary that we advance on the event that audio finishes playing.
        CurrentBuffIndex += 1;
        if (CurrentBuffIndex < AudBuffs.size())
            AdvanceBuffer();
        else
            CanPlayAudio = true;


        if (StdOutput->error() != QAudio::NoError && StdOutput->error() != QAudio::UnderrunError) {
            QMessageBox::critical(this,"Error","Audio stopped playing due to error ID " + QString::number(StdOutput->error()));

        }

    }
}

void MainWindow::OnClipboardDataChanged()
{

    if (ui->actAutoInferClipBrd->isChecked() && !ui->edtInput->hasFocus() && !ClipBrd->text().isEmpty())
    {
        ui->edtInput->setText(ClipBrd->text());
        on_btnInfer_clicked();

    }

}

void MainWindow::OnAttentionRecv(TFTensor<float> InAtt, uint32_t inID)
{
    // The audio recv event should have added the thing to the vec
    InferIDTrueID* pInferEntry = FindByFirst(inID);

    if (pInferEntry)
    {
        Alignments.push_back(InAtt);

        pInferEntry->Align = Alignments.size() - 1;

    }



    if (ui->chkAutoPlay->isChecked() && ui->lstUtts->item((int32_t)inID)->backgroundColor() == PlayingColor)
        PlotAttention(InAtt);





}




void MainWindow::on_btnInfer_clicked()
{
    // While the Voice class can already autoload, we explicitly do so here so we can handle everything (note text, disabling)

    if (VoMan.FindVoice(ui->cbModels->currentText(),false) == -1)
        on_btnLoad_clicked();


    // Convert to lowercase here before we add phonemes
    QString BeforeInput = ui->edtInput->toPlainText();
    QString RawInput = BeforeInput;
    QString Input = RawInput.replace("\n"," ");
    const int MaxShowInputLen = ui->lstUtts->size().width() / 6;



    QStringList InputSplits;
    QStringList BeforeSplits;

    if (ui->rbSplitWord->isChecked())
    {
        BeforeSplits.push_back(Input);

    }
    else{
        BeforeSplits = ui->edtInput->toPlainText().split(QRegExp("[\r\n]"),QString::SkipEmptyParts);



    }
    if (ui->chkPonctAware->isChecked())
    {
        QStringList TempBeforeSplits;
        for (const QString& CuSplit: BeforeSplits)
        {
            TempBeforeSplits.append(CuSplit.split(".",QString::SkipEmptyParts));


        }
        BeforeSplits = TempBeforeSplits;

    }

    for (const QString& LiSplit : BeforeSplits)
    {
        InputSplits.append(SuperWordSplit(LiSplit,ui->spbSeqLen->value()));
    }
    LastInferBatchSize = InputSplits.size();

    for (QString& idvInput : InputSplits)
    {
        if (idvInput.isEmpty())
            continue;

        InferDetails Dets;
        ProcessCurlies(idvInput);

        // If it ends in a period remove it since it will destabilize both Tacotron and FastSpeech
        if (idvInput[idvInput.size() - 1] == '.')
            idvInput = idvInput.left(idvInput.size() - 1);

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
        Dets.Prompt = idvInput;
        Dets.SpeakerID = 0;
        Dets.EmotionID = -1;
        Dets.Denoise = ui->chkDenoise->isChecked();
        Dets.Amplification = (float)ui->sliVolBoost->value() / 1000.f;
        Dets.ExportFileName = "";
        size_t VoiceID = (size_t)VoMan.FindVoice(ui->cbModels->currentText(),true);

        Dets.SampleRate = VoMan[VoiceID]->GetInfo().SampleRate;

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

void MainWindow::PlayBuffer(QBuffer *pBuff,bool ByUser, int32_t RowID)
{





    if (!ui->chkAutoPlay->isChecked() && !ByUser)
        return;



    pBuff->open(QBuffer::ReadWrite);

    QAudioBuffer BuffAud(pBuff->buffer(),StdFmt);

    uint64_t NumSamples = pBuff->size() / sizeof (float);
    const TFTensor<float>& MelSpec = Mels[FindByFirst(RowID)->second];

    if (MelSpec.Shape[0] != -1)
        PlotSpec(MelSpec,( ((float)NumSamples) / ((float)CommonSampleRate)));



    ui->widAudioPlot->setSource(BuffAud);
    ui->widAudioPlot->plot();
    if (ui->actShowWaveform->isChecked())
        ui->tabMetrics->show();



    StdOutput->start(pBuff);
    CanPlayAudio = false;

    ui->lstUtts->item(RowID)->setBackgroundColor(PlayingColor);


    UpdateIfDoSlides();



}

void MainWindow::AdvanceBuffer()
{
    InferIDTrueID* Infer = FindBySecond(CurrentBuffIndex);
    if (Infer->Align != -1)
    {
        PlotAttention(Alignments[(size_t)Infer->Align]);

    }else{
        ui->tabMetrics->setTabEnabled(2,false);
    }

    PlayBuffer(AudBuffs[CurrentBuffIndex],false,FindBySecond(CurrentBuffIndex)->first);
}

void MainWindow::AdvanceQueue()
{
    if (!Infers.size())
        return;


     DoInference(Infers.front());
     Infers.pop();

     ++CurrentInferIndex;



}

int32_t MainWindow::CountBlues()
{

    if (DoingBatchDenoising)
        return 0;


    int32_t NumBlues = 0;
    int32_t NumGreen = 0;

    for (int i = 0; i < ui->lstUtts->count();i++)
    {
        if (!ui->lstUtts->item(i))
            continue;

        if (ui->lstUtts->item(i)->backgroundColor() == InProcessColor)
            NumBlues += 1;
        else if (ui->lstUtts->item(i)->backgroundColor() == DoneColor || ui->lstUtts->item(i)->backgroundColor() == PlayingColor)
            NumGreen += 1;




    }



    // Letting the user click clear when there is an in process utterance will make a crash
    ui->btnClear->setEnabled(NumBlues == 0 && NumGreen == ui->lstUtts->count());



    return NumBlues;

}

int32_t MainWindow::GetNumThreads()
{
    int32_t NThreads = 1;

    if (ui->chkMultiThreaded->isChecked())
        NThreads = ui->spbThreads->value();

    return NThreads;
}

bool MainWindow::MustExplicitlyIterateQueue()
{
    if (!ui->lstUtts->count())
        return true;


    if (std::max(GetNumThreads(),1) < CountBlues())
        return false;

    return true;

}

void MainWindow::PopulateComboBox()
{

    ui->cbModels->clear();
    QStringList CuModels = ListDirs("models");

    if (!CuModels.empty())
        ui->cbModels->insertItems(0,CuModels);

    // This will either disable or enable depending on whether it found models or not.
    ui->cbModels->setEnabled(!CuModels.empty());
    ui->btnLoad->setEnabled(!CuModels.empty());




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

        QString CuWord = RawWords[Idx];

        if (!CuWord.contains("@")) // phonetic input has to be uppercase
            CuWord = CuWord.toLower();

        CurrentStr.append(CuWord);

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


    QRegularExpression& PhonemeExp = pHigh->PhonemeExp;
    QRegularExpressionMatchIterator MatchIter = PhonemeExp.globalMatch(ModTxt);

    while (MatchIter.hasNext()) {
        QRegularExpressionMatch match = MatchIter.next();
        QString ToProcess = ModTxt.mid(match.capturedStart(),match.capturedLength());


        // Curlie processing not supported in IPA
        if (GetCurrentVoice()->GetInfo().s_Language.find("IPA") != std::string::npos)
        {
            QMessageBox::critical((QWidget*)pDarkFw,"Warning","Curly brace phonetic text input processing not supported in IPA");

            return;


        }

        QString SepStr = " ";

        QStringList Toks = ToProcess.split(SepStr,QString::SplitBehavior::SkipEmptyParts,Qt::CaseInsensitive);

        QStringList NewTokens;
        for (QString& Tk : Toks){
            Tk = Tk.replace("{","").replace("}","");
            if (Tk.isEmpty())
                continue;




            // Only English requires all phn input to be uppercase

            if (GetCurrentVoice()->GetInfo().Language == 0)
                Tk = Tk.toUpper();

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

    int32_t NumInfer = GetNumThreads();

    // Count how many we must infer
    // This is the amount of threads minus the amount that are already being done now
    // We ensure this is not negative by forcing it to 0
    NumInfer = std::max(NumInfer - CountBlues(),0);



    if (!NumInfer)
        return;

    for (int32_t t = 0; t < NumInfer;t++)
    {
        if (CurrentAmtThreads >= (uint32_t)GetNumThreads())
            break;

        AdvanceQueue();



    }

    if (!DoingBatchDenoising)
         CountBlues();





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
    VoxThread->ForcedAudio = Dets.ForcedAudio;

    if (!VoxThread->ForcedAudio.size())
    {
        size_t VoiceID = (size_t)VoMan.FindVoice(Dets.VoiceName,true);
        //Auto-load is true, so we will always get a good pointer.
        VoxThread->pAttVoice = VoMan[VoiceID];
    }else
    {
        // Denoising threads don't need voices.
        VoxThread->pAttVoice = nullptr;

    }

    VoxThread->SampleRate = Dets.SampleRate;
    VoxThread->EmotionID = Dets.EmotionID;

    VoxThread->CurrentID = (uint32_t)CurrentInferIndex;
    VoxThread->Denoise = Dets.Denoise;
    VoxThread->Amplify = Dets.Amplification;
    VoxThread->ExportFileName = Dets.ExportFileName;

    connect(VoxThread,&Voxer::Done,this,&MainWindow::OnAudioRecv);
    connect(VoxThread,&Voxer::AttentionReady,this,&MainWindow::OnAttentionRecv);

    // Otherwise the thread lingers and causes a memory leak
    connect(VoxThread, &Voxer::finished, VoxThread, &QObject::deleteLater);


    VoxThread->start();
    ++CurrentAmtThreads;

    ui->btnClear->setEnabled(false);

}



void MainWindow::on_btnLoad_clicked()
{
    LogiLedFlashLighting(55,0,100,LOGI_LED_DURATION_INFINITE,500);
    size_t VoID = VoMan.LoadVoice(ui->cbModels->currentText());
    ui->btnLoad->setEnabled(false);
    std::string VoNote = VoMan[VoID]->GetInfo().Note;

    ui->lblModelNote->setText(QString::fromStdString(VoNote));
    HandleIsMultiSpeaker(VoID);

    LogiLedStopEffects();

    LogiLedFlashLighting(0,100,100,5000,500);


    if (VoMan[VoID]->GetInfo().Architecture.Text2Mel != EText2MelModel::Tacotron2)
        ui->tabMetrics->setTabEnabled(2,false);






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


    if (item->backgroundColor() == DoneColor || item->backgroundColor() == PlayingColor){


        InferIDTrueID* pInfer = FindByFirst(ui->lstUtts->row(item));
        if (!pInfer)
        {
            QMessageBox::critical(this,"Error!!!!","Could not find the inference ID to play, but it's apparently done???? (You shouldn't be seeing this error!!!!) (panik)");
            return;

        }

        InferIDTrueID* PreviousInfer = FindBySecond(CurrentBuffIndex);
        if (PreviousInfer)
        {
            if (ui->lstUtts->item(PreviousInfer->first)->backgroundColor() == PlayingColor)
            {
                ui->lstUtts->item(PreviousInfer->first)->setBackgroundColor(DoneColor);


            }


        }

        QBuffer* pBuff = AudBuffs[(uint64_t)pInfer->second];
        CurrentBuffIndex = pInfer->second;

        PlayBuffer(pBuff,true,pInfer->first);
        if (pInfer->Align != -1)
        {
            PlotAttention(Alignments[(size_t)pInfer->Align]);

        }else{
            ui->tabMetrics->setTabEnabled(2,false);
        }



        int32_t MSecsShow = (pBuff->size() / sizeof (float)) / (int32_t)(CommonSampleRate / 1000);
        LogiLedFlashLighting(100,100,100,MSecsShow,200);

        UpdateIfDoSlides();







    }


}

void MainWindow::on_btnClear_clicked()
{
    ui->lstUtts->clear();

    AudBuffs.clear();
    Alignments.clear();
    Mels.clear();

    CurrentBuffIndex = 0;
    PerfReportLines.clear();
    CurrentInferIndex = 0;

    pTskProg->hide();

    IdVec.clear();
    NumDone = 0;
    CurrentAmtThreads = 0;

    UpdateLogiLed();




}

void MainWindow::on_btnExportSel_clicked()
{
    if (ui->lstUtts->selectedItems().size() == 0){
        QMessageBox::critical(FwParent,"Error","You have to select an item to export selection.");
        return;

    }


    QString ofname = QFileDialog::getSaveFileName(FwParent, tr("Export WAV file"), LastExportDir, tr("WAV, float32 PCM (*.wav)"));
    if (!ofname.size())
        return;

    std::vector<float> Audat;
    QByteArray& AuBuff = AudBuffs[(size_t)ui->lstUtts->currentRow()]->buffer();
    ExportAudBuffer(ofname,AuBuff,CommonSampleRate);

    LastExportDir = ofname;


}

void MainWindow::on_actionExport_performance_report_triggered()
{
    QString ofname = QFileDialog::getSaveFileName(FwParent, tr("Export performance report TXT file"), "log", tr("Text file (*.txt)"));
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

    bool AltExport = GetAsyncKeyState(VK_LSHIFT);


    QString ExTitle = "Export WAV file";

    if (AltExport)
        ExTitle += "s (separately)";

    QString ofname = QFileDialog::getSaveFileName(FwParent,ExTitle,LastExportDir, tr("WAV, float32 PCM (*.wav)"));
    if (!ofname.size())
        return;


    LogiLedSetLighting(100,100,100);

    if (AltExport)
    {
        for (int32_t i = 0; i < ui->lstUtts->count();i++)
        {


            QString FnNoFex = ofname.split(".")[0];
            FnNoFex += QString::number(i);
            FnNoFex += ".wav";

            ExportAudBuffer(FnNoFex,AudBuffs[(uint64_t)GetID(i)]->buffer(),CommonSampleRate);

        }


        LogiLedFlashLighting(0,50,100,5000,500);


        ResetLogiLedIn(8);
        LastExportDir = ofname;

        return;


    }

    QByteArray CuBuff;

    for (int32_t i = 0; i < ui->lstUtts->count();i++)
    {
        CuBuff.append(AudBuffs[(uint64_t)GetID(i)]->buffer());

    }
    ExportAudBuffer(ofname,CuBuff,CommonSampleRate);



    LogiLedFlashLighting(0,50,100,5000,500);

    LastExportDir = ofname;


    ResetLogiLedIn(8);


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

    AutoUpdateSplit();


    ArchitectureInfo Inf = CurrentVoice.GetInfo().Architecture;
    if (Inf.Text2Mel == EText2MelModel::FastSpeech2)
    {
        ui->grpFs2Params->show();
        ui->chkBiPad->setEnabled(true);
    }
    else
    {
        ui->grpFs2Params->hide();
        ui->chkBiPad->setEnabled(false);



    }


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
    int32_t CurrentIndex = VoMan.FindVoice(ui->cbModels->currentText(),false);

    if (CurrentIndex == -1){
        QMessageBox::critical(FwParent,"Error","You gotta have a model loaded before accessing the phonetic dictionary. (We need to know which language you want to use)");
        return;

    }

    if (VoMan[CurrentIndex]->GetInfo().Language < 0){
        QMessageBox::critical(FwParent,"Error","Phonetic overrides dictionary is not available for character-based models. Please use a phoneme-based model.");
        return;


    }

    FramelessWindow FDlg(FwParent);
    FDlg.setWindowIcon(QIcon(":/res/phoneticdico.png"));
    FDlg.setWindowTitle("Phonetic Overrides");
    FDlg.SetTitleBarBtns(false,false,true);
    FDlg.resize(640,480);

    PhdDialog Dlg(FwParent);
    Dlg.Entrs = PhonDict.Entries;
    Dlg.CurrentLang = VoMan[CurrentIndex]->GetInfo().s_Language;

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
        QMessageBox::critical(FwParent,"Error","You have to load the model before accessing its info");
        return;

    }
    VoiceInfo Voi = VoMan[(size_t)CurrentIndex]->GetInfo();

    FramelessWindow FDlg(FwParent);
    FDlg.setWindowIcon(QIcon(":/res/infico.png"));
    FDlg.setWindowTitle("Model Info");
    FDlg.SetTitleBarBtns(false,false,true);
    FDlg.resize(600,550);

    ModelInfoDlg Dlg(FwParent);

    FDlg.setContent(&Dlg);
    FDlg.ContentDlg(&Dlg);

    QString MdlDesc = QString::fromStdString(Voi.Description);
    std::string MiExpanded = VoMan[(size_t)CurrentIndex]->GetModelInfo();

    if (MiExpanded.size() > 1)
        MdlDesc = QString::fromStdString(MiExpanded);


    FDlg.show();
    Dlg.SetInfo(QString::fromStdString(Voi.Name),MdlDesc,Voi.Version,QString::fromStdString(Voi.Author),
                QString::fromStdString(Voi.Architecture.s_Repo),QString::fromStdString(Voi.Architecture.s_Text2Mel),
                QString::fromStdString(Voi.Architecture.s_Vocoder),Voi.SampleRate);


    Dlg.setModal(true);

    Dlg.exec();

}

void MainWindow::on_chkMultiThreaded_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked)
    {
        ui->chkAutoPlay->setChecked(false);
        ui->chkAutoPlay->setEnabled(false);

    }else{
        ui->chkAutoPlay->setEnabled(true);
    }

}

void MainWindow::OnFireLogiLed()
{
    UpdateLogiLed();
}

void MainWindow::ExportAudBuffer(const QString &InFilename, const QByteArray &CurrentBuff, uint32_t InSampleRate)
{


    std::vector<float> Audat;

    Audat.resize((size_t)CurrentBuff.size() / sizeof(float));


    smemcpy(Audat.data(),Audat.size() * sizeof(float),CurrentBuff.data(),(size_t)CurrentBuff.size());


    VoxUtil::ExportWAV(InFilename.toStdString(),Audat,InSampleRate);

}

void MainWindow::ResetLogiLedIn(unsigned secs)
{
   QTimer::singleShot(secs * 1000,this,&MainWindow::OnFireLogiLed);

}

int32_t MainWindow::GetID(int32_t InID)
{
    for (const InferIDTrueID& CuId : IdVec)
    {
        if (CuId.first == (uint32_t)InID)
            return (int32_t)CuId.second;


    }

    return -1;
}

void MainWindow::UpdateLogiLed()
{
    if (!LogiLedAvailable)
        return;


    if (ui->lstUtts->count() == 0)
        LogiLedSetLighting(100, 0, 95);
    else
        LogiLedSetLighting(0,100,50);


}

void MainWindow::on_actDenWAV_triggered()
{

    QString fnamei = QFileDialog::getOpenFileName(this, tr("Open WAV to denoise"), QString(), tr("Wave files (*.wav)"));

    if (fnamei == "")
        return;

    AudioFile<float> AudFile;
    AudFile.load(fnamei.toStdString());


    InferDetails Dets;


    QString RawFn = fnamei.split("/").last();
    QListWidgetItem* widItm = new QListWidgetItem("Denoise " + RawFn,ui->lstUtts);




    Dets.F0 = RangeToFloat(ui->sliF0->value());
    Dets.Speed = RangeToFloat(ui->sliSpeed->value());
    Dets.Energy = RangeToFloat(ui->sliEnergy->value());
    Dets.pItem = widItm;
    Dets.Prompt = "";
    Dets.SpeakerID = 0;
    Dets.EmotionID = -1;
    Dets.Denoise = true;
    Dets.Amplification = (float)ui->sliVolBoost->value() / 1000.f;
    Dets.ExportFileName = "";


    Dets.VoiceName = ui->cbModels->currentText();
    Dets.ForcedAudio = AudFile.samples[0];
    Dets.SampleRate = AudFile.getSampleRate();

    Infers.push(Dets);
    if (MustExplicitlyIterateQueue())
        IterateQueue();

}

void MainWindow::on_actShowWaveform_triggered()
{

}

void MainWindow::on_actShowWaveform_toggled(bool arg1)
{
    if (!arg1)
        ui->tabMetrics->hide();

}

void MainWindow::on_tabMetrics_currentChanged(int index)
{
    if (index == 0)
    {
        ui->tabMetrics->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Preferred);
        ui->tabMetrics->setMinimumHeight(70);



    }
    else
    {
        ui->tabMetrics->setSizePolicy(QSizePolicy::Policy::Expanding,QSizePolicy::Policy::Expanding);
        ui->tabMetrics->setMinimumHeight(150);


    }

    update();

    UpdateIfDoSlides();



}

InferIDTrueID* MainWindow::FindBySecond(int32_t BuffID)
{
    for (auto& ID : IdVec)
    {
        if ((size_t)BuffID == ID.second)
            return &ID;

    }
    return nullptr;
}

void MainWindow::UpdateIfDoSlides()
{
    // Why do we do this instead of letting slides happen regardless?
    // Because due to constant replotting this operation is quite expensive and can cause a noticeable performance drop
    // if we let two of them go around at the same time

    ui->widSpec->DoSlide = ui->tabMetrics->currentIndex() == 1 && StdOutput->state() == QAudio::State::ActiveState;

    ui->widAudioPlot->DoSlide = ui->tabMetrics->currentIndex() == 0 && StdOutput->state() == QAudio::State::ActiveState;

}

void MainWindow::PlotSpec(const TFTensor<float> &InMel,float TimeInSecs)
{
    UpdateIfDoSlides();
    ui->widSpec->DoPlot(InMel,TimeInSecs);

}

void MainWindow::PlotAttention(const TFTensor<float>& TacAtt)
{
    UpdateIfDoSlides();

    ui->tabMetrics->setTabEnabled(2,true);


    ui->widAttention->DoPlot(TacAtt);


}

void MainWindow::on_actExAtt_triggered()
{
    if (!ui->tabMetrics->isTabEnabled(2))
    {
        QMessageBox::critical(FwParent,"Error","There is no attention map to export. Only Tacotron 2 models generate alignment.");
        return;


    }


    QString ofname = QFileDialog::getSaveFileName(FwParent, tr("Export PNG file"), "AttentionMap", tr("PNG image (*.png)"));
    if (!ofname.size())
        return;

    ui->widAttention->savePng(ofname,ui->widAttention->width() * 2,ui->widAttention->height() * 2);


}

void MainWindow::on_actExSpec_triggered()
{

    QString ofname = QFileDialog::getSaveFileName(FwParent, tr("Export PNG file"), "Spect", tr("PNG image (*.png)"));
    if (!ofname.size())
        return;

    ui->widSpec->savePng(ofname,ui->widSpec->width(),ui->widSpec->height());


}

void MainWindow::OnMemoryUpdate()
{
    size_t MemUsg = GetMemoryUsage();

    QString MemStr = QString::number((size_t)(MemUsg / 1e+6)) + " MB";
    if (MemUsg > 1e+9)
        MemStr = QString::number(MemUsg / 1e+9,'f',1) + " GB";


    StatusLbl->setText("Memory usage: " + MemStr);

}

size_t MainWindow::GetMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS MemCtr;

    GetProcessMemoryInfo( GetCurrentProcess(), &MemCtr, sizeof(MemCtr));

    return MemCtr.WorkingSetSize;




}

void MainWindow::on_actionPhonemize_filelist_triggered()
{
    QString fnamei = QFileDialog::getOpenFileName(this, tr("Open TXT to phonemize"), QString(), tr("TXT filelist files (*.txt)"));

    if (fnamei == "")
        return;

    int32_t CurrentIndex = VoMan.FindVoice(ui->cbModels->currentText(),false);

    if (CurrentIndex == -1){
        QMessageBox::critical(FwParent,"Error","You have to load the model before phonemizing");
        return;

    }

    Voice& CurrentVoice = *VoMan[CurrentIndex];

    QFile InputFile(fnamei);
    QFile OutputFile(fnamei.replace(".txt","_p.txt"));

    OutputFile.open(QIODevice::WriteOnly | QIODevice::Text);

    if (InputFile.open(QIODevice::ReadOnly))
    {
       QTextStream InStream(&InputFile);
       QTextStream OutStream(&OutputFile);

       while (!InStream.atEnd())
       {
          QString Line = InStream.readLine().replace("\n","");

          QStringList Splitty = Line.split("|");

          if (Splitty.isEmpty())
              continue;

          QString Transcript = Splitty[1];
          QString Filename = Splitty[0];


          QString NewTranscript = PhonemizeStr(Transcript,CurrentVoice);

          OutStream << QStringList{Filename, NewTranscript}.join("|") << "\n";



       }

       InputFile.close();
       OutputFile.close();
    }





}

QString MainWindow::PhonemizeStr(QString &Text, Voice &VoxIn)
{
    const QString punctuation = ",.;¡!¿?':-";




    QString PhonemedTxt = QString::fromStdString(VoxIn.PhonemizeStr(Text.toLower().toStdString()));



    bool InCurlies = false;

    QString NewPhonemed = "";





    QStringList SplitTrans =  PhonemedTxt.split(" ");


    for (int32_t i = 0; i < SplitTrans.size();i++)
    {

        // Add curly braces to phonemes and exclude them for punctuation.

        if (!punctuation.contains(SplitTrans[i])  && !InCurlies){
            NewPhonemed += " { ";
            InCurlies = true;
        }

        if (punctuation.contains(SplitTrans[i]) && InCurlies){
            NewPhonemed += " } ";
            InCurlies = false;
        }






        NewPhonemed += SplitTrans[i].replace("@","") + " ";





    }
    if (InCurlies)
         NewPhonemed += " } ";

    return NewPhonemed.simplified();

}

void MainWindow::on_actPhnSel_triggered()
{
    int32_t CurrentIndex = VoMan.FindVoice(ui->cbModels->currentText(),false);

    if (CurrentIndex == -1){
        QMessageBox::critical(FwParent,"Error","You have to load the model before phonemizing");
        return;

    }

    Voice& CurrentVoice = *VoMan[CurrentIndex];

    QTextCursor TCursor = ui->edtInput->textCursor();
    if (!TCursor.hasSelection())
        return;

    QString SelTxt = TCursor.selectedText();
    TCursor.insertText(PhonemizeStr(SelTxt,CurrentVoice));




}

void MainWindow::AutoUpdateSplit()
{
    if (!DoUpdateSplitAuto)
        return;

    int32_t CurrentIndex = VoMan.FindVoice(ui->cbModels->currentText(),false);

    if (CurrentIndex == -1)
        return;

    Voice& CurrentVoice = *VoMan[CurrentIndex];

    if (CurrentVoice.GetInfo().Architecture.Text2Mel == EText2MelModel::Tacotron2)
        ui->spbSeqLen->setValue(500);
    else
        ui->spbSeqLen->setValue(180);





}

void MainWindow::on_spbSeqLen_editingFinished()
{
    DoUpdateSplitAuto = false;
}

Voice *MainWindow::GetCurrentVoice()
{
    int32_t CurrentIndex = VoMan.FindVoice(ui->cbModels->currentText(),false);
    return VoMan[(size_t)CurrentIndex];


}

void MainWindow::on_actBatchDen_triggered()
{

    FramelessWindow FDlg(FwParent);
    FDlg.setWindowIcon(QIcon(":/res/infico.png"));
    FDlg.setWindowTitle("Batch Denoiser");
    FDlg.SetTitleBarBtns(false,false,true);
    FDlg.resize(520,300);

    BatchDenoiseDlg Dlg(FwParent);

    FDlg.setContent(&Dlg);
    FDlg.ContentDlg(&Dlg);

    Dlg.pMainWindow = this;

    ui->chkAutoPlay->setChecked(false);
    ui->chkMultiThreaded->setChecked(true);

    on_btnClear_clicked();


    FDlg.show();

    Dlg.setModal(true);

    Dlg.exec();

}

void MainWindow::on_actStopPlaying_triggered()
{
    StdOutput->stop();
    UpdateIfDoSlides();

}

bool MainWindow::AllowedToPlayAudio()
{
    return CanPlayAudio && (StdOutput->state() == QAudio::State::IdleState || StdOutput->state() == QAudio::State::StoppedState);

}
