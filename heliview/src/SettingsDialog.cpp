// -----------------------------------------------------------------------------
// File:    SettingsDialog.h
// Authors: Garrett Smith, Kevin Macksamie, Tyler Thierolf, Timothy Miller
// Created: 10-04-2010
//
// General purpose settings dialog window.
// -----------------------------------------------------------------------------

#include "SettingsDialog.h"
#include <QColorDialog>
#include <QFileDialog>
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

    //TODO: Set LogFile Value
    editLogFileName->setText(logfile);
    sbLogBuffer->setValue(logbufsize);
    //TODO: Set RGB Values
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
            sbHt->value(), sbSt->value());

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

