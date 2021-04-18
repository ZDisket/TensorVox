#ifndef PHDDIALOG_H
#define PHDDIALOG_H

#include <QDialog>
#include "phoneticdict.h"
namespace Ui {
class PhdDialog;
}

class PhdDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PhdDialog(QWidget *parent = nullptr);
    ~PhdDialog();

    int exec() override;
    void accept() override;

    std::vector<DictEntry> Entrs;


    std::string CurrentLang;
private slots:
    void on_btnAdd_clicked();

    void on_btnRemove_clicked();

    void on_btnImport_clicked();

    void on_tbDict_cellChanged(int row, int column);

private:
    void PopulateWithEntries();
    Ui::PhdDialog *ui;
};

#endif // PHDDIALOG_H
