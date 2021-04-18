#include "phddialog.h"
#include "ui_phddialog.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
PhdDialog::PhdDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PhdDialog)
{
    ui->setupUi(this);
    ui->tbDict->horizontalHeader()->setStretchLastSection(true);

}

PhdDialog::~PhdDialog()
{
    delete ui;
}

int PhdDialog::exec()
{
    // Populate the list

    PopulateWithEntries();


  //  ui->tbDict->setColumnWidth(0,ui->tbDict->width() / 2);
   // ui->tbDict->setColumnWidth(1,ui->tbDict->width() / 2);
    return QDialog::exec();
}

void PhdDialog::accept()
{
    // Validate input
    for (int i = 0; i < ui->tbDict->rowCount();++i)
    {
        if (ui->tbDict->item(i,0)->text().isEmpty()
            || ui->tbDict->item(i,0)->text() == " " ||
            ui->tbDict->item(i,1)->text().isEmpty())
        {
            QMessageBox::critical(this,"Invalid input","None of the cells can be empty, and words cannot be spaces. Check your input and try again.");
            return;

        }


    }

    // Now clear and run second loop

    Entrs.clear();
    Entrs.reserve((size_t)ui->tbDict->rowCount());
    // Second loop
    for (int i = 0; i < ui->tbDict->rowCount();++i)
    {
        DictEntry de;
        de.Word = ui->tbDict->item(i,0)->text().toStdString();
        de.PhSpelling = ui->tbDict->item(i,1)->text().toStdString();
        de.Language = ui->tbDict->item(i,2)->text().toStdString();
        Entrs.push_back(de);



    }

    QDialog::accept();
}

void PhdDialog::on_btnAdd_clicked()
{
    ui->tbDict->insertRow(ui->tbDict->rowCount());
    ui->tbDict->scrollToItem(ui->tbDict->item(ui->tbDict->rowCount() - 1,0));

    QTableWidgetItem* LangItem = new QTableWidgetItem(QString::fromStdString(CurrentLang));
    LangItem->setFlags(LangItem->flags() ^ Qt::ItemIsEditable);

    ui->tbDict->setItem(ui->tbDict->rowCount() - 1,2,LangItem);

}

void PhdDialog::PopulateWithEntries()
{
    ui->tbDict->clearContents();
    ui->tbDict->setRowCount((int)Entrs.size());
    for (size_t i = 0;i < Entrs.size();++i)
    {
        ui->tbDict->setItem((int)i,0,new QTableWidgetItem(QString::fromStdString(Entrs[i].Word)));
        ui->tbDict->setItem((int)i,1,new QTableWidgetItem(QString::fromStdString(Entrs[i].PhSpelling)));

        QTableWidgetItem* LangItem = new QTableWidgetItem(QString::fromStdString(Entrs[i].Language));
        LangItem->setFlags(LangItem->flags() ^ Qt::ItemIsEditable);

        ui->tbDict->setItem((int)i,2,LangItem);


    }

}

void PhdDialog::on_btnRemove_clicked()
{
    QList<QTableWidgetItem*> seli = ui->tbDict->selectedItems();
    QList<QTableWidgetItem*>::iterator It = seli.begin();
    while (It != seli.end())
    {
        QTableWidgetItem* item = *It;
        ui->tbDict->removeRow(item->row());

        ++It;
    }
}

void PhdDialog::on_btnImport_clicked()
{
    QString fnamei = QFileDialog::getOpenFileName(this, tr("Open dictionary to import"), QString(), tr("DeltaVox Phonetic Dictionary Files (*.phd)"));

    if (fnamei == "")
        return;

    PhoneticDict Pd;
    if (!Pd.Import(fnamei)){
        QMessageBox::critical(this,"Error","Failed to import this file.");
        return;
    }

    Entrs.reserve(Entrs.size() + Pd.Entries.size());
    for (DictEntry& De : Pd.Entries )
    {
        Entrs.push_back(De);


    }
    PopulateWithEntries();



}

void PhdDialog::on_tbDict_cellChanged(int row, int column)
{
    if (row != 0)
        return;

}
