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
    SettingsDialog(QWidget *parent, bool track_en, bool track_btn_en,
            const TrackSettings &track, const QString &logfile, const int logbufsize);
    virtual ~SettingsDialog();

signals:
    void updateColorTrackEnable(int track_en);
    void trackSettingsChanged(int r, int g, int b, int ht, int st, int ft, int fps);
    void logSettingsChanged(const QString &, const QString &, int);
    void deviceControlChanged(int id, int value);
    void trimSettingsChanged(int axis, int value);
    void filterSettingsChanged(int signal, int samples);
    void pidSettingsChanged(int axis, int signal, float value);
    void videoRotationChanged(int angle);
    
public slots:
    // device control related events
    void onDeviceControlUpdated(const QString &name, const QString &type,
            int id, int minimum, int maximum, int step, int default_value,
            int current_value);
    void onDeviceMenuUpdated(const QString &name, int id, int index);
    void onTrimSettingsUpdated(int yaw, int pitch, int roll, int thro);
    void onFilterSettingsUpdated(int imu, int alt, int aux, int batt);
    void onColorValuesUpdated(TrackSettings track);
    void onPIDSettingsUpdated(int axis, float p, float i, float d, float set);
    void onUpdateColorTrackEnable(int enabled);
    void onRotationIndexChanged(int index);

    void onDeviceControlCheckStateChanged(int state);
    void onDeviceControlSliderValueChanged(int value);
    void onDeviceControlMenuItemChanged(int index);

    // button click events
    void onBrowseClicked();
    void onCancelClicked();
    void onOkClicked();
    void onApplyClicked();
    void onNewColorClicked();
    void onColorTrackingClicked();

    // slider track events
    void onTrimSliderChanged(int value);
    void onFilterSliderChanged(int value);

    // double spin box events
    void onPIDSpinBoxChanged();
    
protected:
    int m_devctrls;
    int m_rotation;
    QMap<QObject *, int> m_dev_to_id;
    QMap<int, QObject *> m_id_to_dev;
    bool                 m_colortrack_en;
};

#endif // _HELIVIEW_SETTINGSDIALOG__H_

