#include "SettingsDialog.h"
#include <QColorDialog>
#include "DeviceController.h"

SettingsDialog::SettingsDialog(QWidget *parent,DeviceController * controller,
                                int *_r, int *_g, int *_b, int *_ht, int *_st)
: QDialog(parent), m_controller(controller), r(_r), g(_g), b(_b), ht(_ht), st(_st)
{
    setupUi(this);
    //Primary Buttons
    connect(btnCancel, SIGNAL(released()), this, SLOT(s_cancelButton()));
    connect(btnOK, SIGNAL(released()), this, SLOT(s_okButton()));
    connect(btnApply, SIGNAL(released()), this, SLOT(s_applyButton()));
    
    connect(btnNewColor, SIGNAL(released()), this, SLOT(s_newColorButton()));
    sbR->setValue(*_r);
    sbG->setValue(*_g);
    sbB->setValue(*_b);
    sbHt->setValue(*_ht);
    sbSt->setValue(*_st);

    //TODO: Set LogFile Value
    //TODO: Set RGB Values
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::s_cancelButton(){
    reject();
}
void SettingsDialog::s_okButton(){
    s_applyButton();
    accept();
}
void SettingsDialog::s_applyButton(){
    m_controller->onUpdateTrackColor(sbR->value(), sbG->value(), sbB->value(), 
        sbHt->value(), sbSt->value());
    *r = sbR->value();
    *g = sbG->value();
    *b = sbB->value();
    *ht = sbHt->value();
    *st = sbSt->value();
}

void SettingsDialog::s_newColorButton(){
    QColor color = QColorDialog::getColor(
        QColor(sbR->value(), sbG->value(), sbB->value(),0), this);
    
    sbR->setValue(color.red());
    sbG->setValue(color.blue());
    sbB->setValue(color.green());
       
    
}
