#include "SettingsDialog.h"
#include <QColorDialog>

SettingsDialog::SettingsDialog(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);
    //Primary Buttons
    connect(btnCancel, SIGNAL(released()), this, SLOT(s_cancelButton()));
    connect(btnOK, SIGNAL(released()), this, SLOT(s_okButton()));
    connect(btnApply, SIGNAL(released()), this, SLOT(s_applyButton()));
    
    connect(btnNewColor, SIGNAL(released()), this, SLOT(s_newColorButton()));
    
    
    //TODO: Set LogFile Value
    //TODO: Set RGB Values
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::s_cancelButton(){
     //printf("CancelButton\n");

    reject();
}
void SettingsDialog::s_okButton(){
     //printf("OK Button\n");
    s_applyButton();

    accept();
}
void SettingsDialog::s_applyButton(){
     //printf("Apply Button\n");

    
}

void SettingsDialog::s_newColorButton(){
     //printf("NewColor Button\n");
    //QColorDialog cd = QColorDialog(
    //    QColor(sbR->value(), sbG->value(), sbB->value(),0), this);
    //cd.setOption(QColorDialog::ShowAlphaChannel,false);
    //cd.open();
    QColor color = QColorDialog::getColor(QColor(sbR->value(), sbG->value(), sbB->value(),0), this);
    
    sbR->setValue(color.red());
    sbG->setValue(color.blue());
    sbB->setValue(color.green());
       
    
}
