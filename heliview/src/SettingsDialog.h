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
    void updateDeviceControl(int id, int value);
    void updateTracking(int r, int g, int b, int ht, int st, int ft);
    void updateLogFile(const QString &, int);
    void updateAxisTrim(int axes, int value);
    void updateSignalFilter(int filter, int samples);
    
public slots:
    // device control related events
    void onDeviceControlUpdate(const QString &name, const QString &type,
            int id, int minimum, int maximum, int step, int default_value);
    void onDeviceMenuUpdate(const QString &name, int id, int index);
    void onDeviceControlCheckStateChanged(int state);
    void onDeviceControlSliderValueChanged(int value);
    void onDeviceControlMenuItemChanged(int index);

    // button click events
    void onBrowseClicked();
    void onCancelClicked();
    void onOkClicked();
    void onApplyClicked();
    void onNewColorClicked();
    void onColorTrackingEnabled(bool enabled);

    // slider track events
    void onTrimSliderChanged(int value);
    void onFilterSliderChanged(int value);

protected:
    int m_devctrls;
    QMap<QObject *, int> m_dev_to_id;
    QMap<int, QObject *> m_id_to_dev;
};

#endif // _HELIVIEW_SETTINGSDIALOG__H_

