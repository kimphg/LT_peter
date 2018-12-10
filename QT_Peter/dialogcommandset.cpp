#include "dialogcommandset.h"
#include "ui_dialogcommandset.h"

DialogCommandSet::DialogCommandSet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCommandSet)
{
    ui->setupUi(this);

    ui->groupBox_control_setting->setHidden(false);
    ui->plainTextEdit_range_8->setPlainText (       CConfig::getString("mR0Command"));
    ui->plainTextEdit_range_9->setPlainText (       CConfig::getString("mR1Command"));
    ui->plainTextEdit_range_10->setPlainText(       CConfig::getString("mR2Command"));
    ui->plainTextEdit_range_11->setPlainText(       CConfig::getString("mR3Command"));
    ui->plainTextEdit_range_12->setPlainText(       CConfig::getString("mR4Command"));
    ui->plainTextEdit_range_13->setPlainText(       CConfig::getString("mR5Command"));
    ui->plainTextEdit_range_14->setPlainText(       CConfig::getString("mR6Command"));
    ui->plainTextEdit_range_15->setPlainText(       CConfig::getString("mR7Command"));
    ui->plainTextEdit_range_16->setPlainText(       CConfig::getString("mR8Command"));
    ui->plainTextEdit_command_rx_2->setPlainText(   CConfig::getString("mRxCommand"));
    ui->plainTextEdit_command_tx_2->setPlainText(   CConfig::getString("mTxCommand"));
    ui->plainTextEdit_freq_7->setPlainText( CConfig::getString("mFreq1Command"));
    ui->plainTextEdit_freq_8->setPlainText( CConfig::getString("mFreq2Command"));
    ui->plainTextEdit_freq_9->setPlainText( CConfig::getString("mFreq3Command"));
    ui->plainTextEdit_freq_10->setPlainText(CConfig::getString("mFreq4Command"));
    ui->plainTextEdit_freq_11->setPlainText(CConfig::getString("mFreq5Command"));
    ui->plainTextEdit_freq_12->setPlainText(CConfig::getString("mFreq6Command"));
    ui->plainTextEdit_freq_13->setPlainText(CConfig::getString("mFreq7Command"));
    ui->plainTextEdit_freq_14->setPlainText(CConfig::getString("mFreq8Command"));
    ui->plainTextEdit_freq_15->setPlainText(CConfig::getString("mFreq9Command"));
    ui->plainTextEdit_freq_16->setPlainText(CConfig::getString("mFreq10Command"));
    ui->plainTextEdit_freq_17->setPlainText(CConfig::getString("mFreq11Command"));
    ui->plainTextEdit_freq_18->setPlainText(CConfig::getString("mFreq12Command"));
}

DialogCommandSet::~DialogCommandSet()
{
    delete ui;
}

void DialogCommandSet::on_buttonBox_accepted()
{
    /*mR0Command = ui->plainTextEdit_range_0->toPlainText();
    mR1Command = ui->plainTextEdit_range_1->toPlainText();
    mR2Command = ui->plainTextEdit_range_2->toPlainText();
    mR3Command = ui->plainTextEdit_range_3->toPlainText();
    mR4Command = ui->plainTextEdit_range_4->toPlainText();
    mR5Command = ui->plainTextEdit_range_5->toPlainText();
    mR6Command = ui->plainTextEdit_range_6->toPlainText();
    mR7Command = ui->plainTextEdit_range_7->toPlainText();

    mRxCommand = ui->plainTextEdit_command_rx->toPlainText();
    mTxCommand = ui->plainTextEdit_command_tx->toPlainText();
    CConfig::setValue("mR0Command",mR0Command);
    CConfig::setValue("mR1Command",mR1Command);
    CConfig::setValue("mR2Command",mR2Command);
    CConfig::setValue("mR3Command",mR3Command);
    CConfig::setValue("mR4Command",mR4Command);
    CConfig::setValue("mR5Command",mR5Command);
    CConfig::setValue("mR6Command",mR6Command);
    CConfig::setValue("mR7Command",mR7Command);
    CConfig::setValue("mRxCommand",mRxCommand);
    CConfig::setValue("mTxCommand",mTxCommand);
    //
    mFreq1Command =  ui->plainTextEdit_freq_1->toPlainText();
    mFreq2Command =  ui->plainTextEdit_freq_2->toPlainText();
    mFreq3Command =  ui->plainTextEdit_freq_3->toPlainText();
    mFreq4Command =  ui->plainTextEdit_freq_4->toPlainText();
    mFreq5Command =  ui->plainTextEdit_freq_5->toPlainText();
    mFreq6Command =  ui->plainTextEdit_freq_6->toPlainText();
    //
    CConfig::setValue("mFreq1Command",mFreq1Command);
    CConfig::setValue("mFreq2Command",mFreq2Command);
    CConfig::setValue("mFreq3Command",mFreq3Command);
    CConfig::setValue("mFreq4Command",mFreq4Command);
    CConfig::setValue("mFreq5Command",mFreq5Command);
    CConfig::setValue("mFreq6Command",mFreq6Command);
    ui->groupBox_control_setting->setHidden(true);
    ui->toolButton_set_commands->setChecked(false);*/

}

void DialogCommandSet::on_pushButton_save_clicked()
{
    QString mR0Command = ui->plainTextEdit_range_8->toPlainText();
    QString mR1Command = ui->plainTextEdit_range_9->toPlainText();
    QString mR2Command = ui->plainTextEdit_range_10->toPlainText();
    QString mR3Command = ui->plainTextEdit_range_11->toPlainText();
    QString mR4Command = ui->plainTextEdit_range_12->toPlainText();
    QString mR5Command = ui->plainTextEdit_range_13->toPlainText();
    QString mR6Command = ui->plainTextEdit_range_14->toPlainText();
    QString mR7Command = ui->plainTextEdit_range_15->toPlainText();
    QString mR8Command = ui->plainTextEdit_range_16->toPlainText();

    QString mRxCommand = ui->plainTextEdit_command_rx_2->toPlainText();
    QString mTxCommand = ui->plainTextEdit_command_tx_2->toPlainText();
    CConfig::setValue("mR0Command",mR0Command);
    CConfig::setValue("mR1Command",mR1Command);
    CConfig::setValue("mR2Command",mR2Command);
    CConfig::setValue("mR3Command",mR3Command);
    CConfig::setValue("mR4Command",mR4Command);
    CConfig::setValue("mR5Command",mR5Command);
    CConfig::setValue("mR6Command",mR6Command);
    CConfig::setValue("mR7Command",mR7Command);
    CConfig::setValue("mR8Command",mR8Command);
    CConfig::setValue("mRxCommand",mRxCommand);
    CConfig::setValue("mTxCommand",mTxCommand);
    //
    QString mFreq1Command =  ui->plainTextEdit_freq_7->toPlainText();
    QString mFreq2Command =  ui->plainTextEdit_freq_8->toPlainText();
    QString mFreq3Command =  ui->plainTextEdit_freq_9->toPlainText();
    QString mFreq4Command =  ui->plainTextEdit_freq_10->toPlainText();
    QString mFreq5Command =  ui->plainTextEdit_freq_11->toPlainText();
    QString mFreq6Command =  ui->plainTextEdit_freq_12->toPlainText();
    QString mFreq7Command =  ui->plainTextEdit_freq_13->toPlainText();
    QString mFreq8Command =  ui->plainTextEdit_freq_14->toPlainText();
    QString mFreq9Command =  ui->plainTextEdit_freq_15->toPlainText();
    QString mFreq10Command =  ui->plainTextEdit_freq_16->toPlainText();
    QString mFreq11Command =  ui->plainTextEdit_freq_17->toPlainText();
    QString mFreq12Command =  ui->plainTextEdit_freq_18->toPlainText();
    //
    CConfig::setValue("mFreq1Command",mFreq1Command);
    CConfig::setValue("mFreq2Command",mFreq2Command);
    CConfig::setValue("mFreq3Command",mFreq3Command);
    CConfig::setValue("mFreq4Command",mFreq4Command);
    CConfig::setValue("mFreq5Command",mFreq5Command);
    CConfig::setValue("mFreq6Command",mFreq6Command);

    CConfig::setValue("mFreq7Command",mFreq7Command);
    CConfig::setValue("mFreq8Command",mFreq8Command);
    CConfig::setValue("mFreq9Command",mFreq9Command);
    CConfig::setValue("mFreq10Command",mFreq10Command);
    CConfig::setValue("mFreq11Command",mFreq11Command);
    CConfig::setValue("mFreq12Command",mFreq12Command);
    CConfig::SaveToFile();

}
