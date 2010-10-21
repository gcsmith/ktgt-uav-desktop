// -----------------------------------------------------------------------------
// File:    SettingsDialog.h
// Authors: Garrett Smith, Kevin Macksamie, Tyler Thierolf, Timothy Miller
// Created: 10-04-2010
//
// General purpose settings dialog window.
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
    void updateTracking(int r, int g, int b, int ht, int st, int ft);
    void updateExposure(int automatic, int value);
    void updateFocus(int automatic, int value);
    void updateWhiteBalance(int automatic);
    void updateLogFile(const QString &, int);
    
public slots:
    void onBrowseClicked();
    void onCancelClicked();
    void onOkClicked();
    void onApplyClicked();
    void onNewColorClicked();
    void onSetCurrentWhiteBalanceClicked();
    void onColorTrackingEnabled(bool enabled);
    void onManualExposureEnabled(bool enabled);
    void onManualFocusEnabled(bool enabled);
    void onManualWhiteBalanceEnabled(bool enabled);
    void onExposureSliderValueChanged(int value);
    void onFocusSliderValueChanged(int value);
};

#endif // _HELIVIEW_SETTINGSDIALOG__H_

