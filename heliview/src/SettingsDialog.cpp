// -----------------------------------------------------------------------------
// File:    SettingsDialog.h
// Authors: Garrett Smith, Kevin Macksamie, Tyler Thierolf, Timothy Miller
// Created: 10-04-2010
//
// General purpose settings dialog window.
// -----------------------------------------------------------------------------

#include <QColorDialog>
#include <QFileDialog>
#include <QCheckBox>
#include <QComboBox>
#include "SettingsDialog.h"
#include "DeviceController.h"
#include "Logger.h"

// -----------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget *pp, const TrackSettings &track, 
        const QString &logfile, const int logbufsize)
: QDialog(pp), m_devctrls(0)
{
    setupUi(this);

    // add a vertical spacer at some absurdly high row index
    ((QGridLayout *)deviceControlScrollAreaContents->layout())->addItem(
        new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding),
        99999, 0, 1, 1);

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
void SettingsDialog::onDeviceControlUpdate(const QString &name,
        const QString &type, int id, int minimum, int maximum, int step,
        int default_value, int current_value)
{
    QWidget *parent = deviceControlScrollAreaContents, *child;
    QGridLayout *gl = (QGridLayout *)parent->layout();

    lblNoDeviceControls->setVisible(false);

    if (type == "bool")
    {
        // add a checkable box (with device name) on the right hand side
        QCheckBox *cb = new QCheckBox(name, parent);
        if(current_value == 1)
            cb->setCheckState((Qt::CheckState)2);
        else
            cb->setCheckState((Qt::CheckState)current_value);
        connect(cb, SIGNAL(stateChanged(int)),
                this, SLOT(onDeviceControlCheckStateChanged(int)));

        gl->addWidget(cb, m_devctrls, 1);
        child = cb;
    }
    else if (type == "int")
    {
        // put a slider of the specified range on the right hand side
        QSlider *slider = new QSlider(Qt::Horizontal, parent);
        slider->setRange(minimum, maximum);
        slider->setSingleStep(step);
        slider->setSliderPosition(current_value);
        connect(slider, SIGNAL(valueChanged(int)),
                this, SLOT(onDeviceControlSliderValueChanged(int)));

        // put a label on the far right side to display the integer value
        QLabel *label = new QLabel(tr("%1").arg(current_value), parent);
        label->setMinimumWidth(40);
        connect(slider, SIGNAL(valueChanged(int)), label, SLOT(setNum(int)));

        gl->addWidget(new QLabel(name, parent), m_devctrls, 0);
        gl->addWidget(slider, m_devctrls, 1);
        gl->addWidget(label, m_devctrls, 2);
        child = slider;
    }
    else if (type == "menu")
    {
        // put a drop down menu on the right hand side
        QComboBox *cb = new QComboBox(parent);
        for(int i=0; i <= maximum; i++){
            cb->addItem("");
        }
        cb->setCurrentIndex(current_value);
        connect(cb, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onDeviceControlMenuItemChanged(int)));

        gl->addWidget(new QLabel(name, parent), m_devctrls, 0);
        gl->addWidget(cb, m_devctrls, 1);
        child = cb;
    }

    m_dev_to_id.insert(child, id);
    m_id_to_dev.insert(id, child);
    m_devctrls++;
}

// -----------------------------------------------------------------------------
void SettingsDialog::onDeviceMenuUpdate(const QString &name, int id, int index)
{
    if (!m_id_to_dev.contains(id))
    {
        Logger::fail("onDeviceMenuUpdate: device control id doesn't exist\n");
        return;
    }

    QComboBox *cb = (QComboBox *)m_id_to_dev.value(id);
    cb->setItemText(index, name);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onDeviceControlCheckStateChanged(int state)
{
    if (!m_dev_to_id.contains(sender()))
    {
        Logger::fail("onDeviceControlCheckStateChanged: invalid sender\n");
        return;
    }

    int id = m_dev_to_id.value(sender());
    emit updateDeviceControl(id, (state == Qt::Checked) ? 1 : 0);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onDeviceControlSliderValueChanged(int value)
{
    if (!m_dev_to_id.contains(sender()))
    {
        Logger::fail("onDeviceControlSliderValueChanged: invalid sender\n");
        return;
    }

    int id = m_dev_to_id.value(sender());
    emit updateDeviceControl(id, value);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onDeviceControlMenuItemChanged(int index)
{
    if (!m_dev_to_id.contains(sender()))
    {
        Logger::fail("onDeviceControlMenuItemChanged: invalid sender\n");
        return;
    }

    int id = m_dev_to_id.value(sender());
    emit updateDeviceControl(id, index);
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
void SettingsDialog::onColorTrackingEnabled(bool enabled)
{
}

// -----------------------------------------------------------------------------
void SettingsDialog::onTrimSliderChanged(int value)
{
    if (sender() == slideThrottleTrim)
        emit updateAxisTrim(AXIS_ALT, value);
    else if (sender() == slideYawTrim)
        emit updateAxisTrim(AXIS_YAW, value);
    else if (sender() == slidePitchTrim)
        emit updateAxisTrim(AXIS_PITCH, value);
    else if (sender() == slideRollTrim)
        emit updateAxisTrim(AXIS_ROLL, value);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onFilterSliderChanged(int value)
{
    if (sender() == slideOrientationFilter)
        emit updateSignalFilter(FILTER_ORIENTATION, value);
    if (sender() == slideAltitudeFilter)
        emit updateSignalFilter(FILTER_ALTITUDE, value);
    if (sender() == slideBatteryFilter)
        emit updateSignalFilter(FILTER_BATTERY, value);
}

