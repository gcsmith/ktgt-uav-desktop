// -----------------------------------------------------------------------------
// File:    SettingsDialog.h
// Authors: Garrett Smith, Kevin Macksamie, Tyler Thierolf, Timothy Miller
// Created: 10-04-2010
//
// General purpose settings dialog window.
// -----------------------------------------------------------------------------

#include <QColorDialog>
#include <QFileDialog>
#include "SettingsDialog.h"
#include "DeviceController.h"

// -----------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget *pp, const TrackSettings &track, 
        const QString &logfile, const int logbufsize)
: QDialog(pp)
{
    setupUi(this);

    // connect primary button events
    connect(btnLogFile, SIGNAL(released()), this, SLOT(onBrowseClicked()));
    connect(btnCancel, SIGNAL(released()), this, SLOT(onCancelClicked()));
    connect(btnOK, SIGNAL(released()), this, SLOT(onOkClicked()));
    connect(btnApply, SIGNAL(released()), this, SLOT(onApplyClicked()));
    connect(btnNewColor, SIGNAL(released()), this, SLOT(onNewColorClicked()));

    // assign specified color and threshold values to dialog widgets
    sbR->setValue(track.color.red());
    sbG->setValue(track.color.green());
    sbB->setValue(track.color.blue());
    sbHt->setValue(track.ht);
    sbSt->setValue(track.st);
    sbFt->setValue(track.ft);

    //TODO: Set LogFile Value
    editLogFileName->setText(logfile);
    sbLogBuffer->setValue(logbufsize);
}

// -----------------------------------------------------------------------------
SettingsDialog::~SettingsDialog()
{
}

// -----------------------------------------------------------------------------
void SettingsDialog::onBrowseClicked()
{
    QStringList filename;

    QFileDialog fd(this);
    fd.setFileMode(QFileDialog::AnyFile);
    fd.exec();

    filename = fd.selectedFiles();
    editLogFileName->setText(filename.at(0));
}

// -----------------------------------------------------------------------------
void SettingsDialog::onCancelClicked()
{
    reject();
}

// -----------------------------------------------------------------------------
void SettingsDialog::onOkClicked()
{
    onApplyClicked();
    accept();
}

// -----------------------------------------------------------------------------
void SettingsDialog::onApplyClicked()
{
    emit updateTracking(sbR->value(), sbG->value(), sbB->value(),
            sbHt->value(), sbSt->value(), sbFt->value());

    emit updateLogFile(editLogFileName->text(), sbLogBuffer->value());
}

// -----------------------------------------------------------------------------
void SettingsDialog::onNewColorClicked()
{
    QColor initial(sbR->value(), sbG->value(), sbB->value());
    QColor color = QColorDialog::getColor(initial, this);
    
    if (color.isValid())
    {
        sbR->setValue(color.red());
        sbG->setValue(color.green());
        sbB->setValue(color.blue());
    }
}

// -----------------------------------------------------------------------------
void SettingsDialog::onSetCurrentWhiteBalanceClicked()
{
    emit updateWhiteBalance(0);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onColorTrackingEnabled(bool enabled)
{
}

// -----------------------------------------------------------------------------
void SettingsDialog::onManualExposureEnabled(bool enabled)
{
    emit updateExposure(enabled ? 0 : 1, 0);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onManualFocusEnabled(bool enabled)
{
    emit updateFocus(enabled ? 0 : 1, 0);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onManualWhiteBalanceEnabled(bool enabled)
{
    emit updateWhiteBalance(enabled ? 0 : 1);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onExposureSliderValueChanged(int value)
{
    emit updateExposure(0, value);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onFocusSliderValueChanged(int value)
{
    emit updateFocus(0, value);
}

