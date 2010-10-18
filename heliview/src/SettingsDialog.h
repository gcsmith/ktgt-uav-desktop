// -----------------------------------------------------------------------------
// File:    SettingsDialog.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_SETTINGSDIALOG__H_
#define _HELIVIEW_SETTINGSDIALOG__H_

#include <QWidget>
#include "ui_SettingsDialog.h"
#include "DeviceController.h"

class SettingsDialog : public QDialog, public Ui::SettingsDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent, int r, int g, int b, int ht, int st);
    virtual ~SettingsDialog();

signals:
    void updateTracking(int r, int g, int b, int ht, int st);
    
public slots:
    void onCancelClicked();
    void onOkClicked();
    void onApplyClicked();
    void onNewColorClicked();

private:
    int m_r, m_g, m_b, m_ht, m_st;   
};

#endif // _HELIVIEW_SETTINGSDIALOG__H_

