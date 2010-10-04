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
};

#endif // _HELIVIEW_SETTINGSDIALOG__H_

