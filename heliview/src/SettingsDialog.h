// -----------------------------------------------------------------------------
// File:    SettingsDialog.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_SETTINGSDIALOG__H_
#define _HELIVIEW_SETTINGSDIALOG__H_

#include <QWidget>
#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog, public Ui::SettingsDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent);
    virtual ~SettingsDialog();
    
public slots:
    void s_cancelButton();
    void s_okButton();
    void s_applyButton();
    
    void s_newColorButton();
    
    
};

#endif // _HELIVIEW_SETTINGSDIALOG__H_

