#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QDirIterator>
#include <QSplitter>
#include <QAudioOutput>
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


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnAudioRecv(std::vector<float> InDat)
{

    QBuffer* Buf = new QBuffer(this);
    Buf->setData((const char*)InDat.data(),sizeof(float) * InDat.size());


    AudBuffs.push_back(Buf);
    PlayBuffer(Buf);
}


void MainWindow::on_btnInfer_clicked()
{
    QString Input = ui->edtInput->toPlainText();
    QListWidgetItem* widItm = new QListWidgetItem(Input,ui->lstUtts);

    Voxer* VoxThread = new Voxer;
    VoxThread->F0 = 1.f;
    VoxThread->Energy = RangeToFloat(ui->sliEnergy->value());
    VoxThread->Speed = RangeToFloat(ui->sliSpeed->value());
    VoxThread->Prompt = Input;
    VoxThread->pAttItem = widItm;
    VoxThread->SpeakerID = 0;
    //Auto-load is true, so we will always get a good pointer.
    VoxThread->pAttVoice = VoMan[(size_t)VoMan.FindVoice(ui->cbModels->currentText(),true)];

    connect(VoxThread,&Voxer::Done,this,&MainWindow::OnAudioRecv);

    VoxThread->start();
    ui->btnLoad->setEnabled(false);

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
    cout << "Audio rev" << endl;

    QAudioOutput* Output = new QAudioOutput(StdFmt,this);
    pBuff->open(QBuffer::ReadWrite);

    Output->start(pBuff);


}

void MainWindow::PopulateComboBox()
{

    ui->cbModels->insertItems(0,ListDirs("models"));


}



void MainWindow::on_btnLoad_clicked()
{
    VoMan.LoadVoice(ui->cbModels->currentText());
    ui->btnLoad->setEnabled(false);

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
