// -----------------------------------------------------------------------------
// File:    SettingsDialog.h
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#ifndef _HELIVIEW_SETTINGSDIALOG__H_
#define _HELIVIEW_SETTINGSDIALOG__H_

#include <QWidget>
#include "ui_SettingsDialog.h"
#include "DeviceController.h"

class SettingsDialog : public QDialog, protected Ui::SettingsDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent, const TrackSettings &track, 
            const QString &logfile, const int logbufsize);
    virtual ~SettingsDialog();

signals:
    void updateTracking(int r, int g, int b, int ht, int st);
    void updateLogFile(const QString &, int);
    
public slots:
    void onBrowseClicked();
    void onCancelClicked();
    void onOkClicked();
    void onApplyClicked();
    void onNewColorClicked();
};

#endif // _HELIVIEW_SETTINGSDIALOG__H_

