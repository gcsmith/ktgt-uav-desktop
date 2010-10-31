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
#include "uav_protocol.h"

// -----------------------------------------------------------------------------
SettingsDialog::SettingsDialog(QWidget *pp, bool track_en, bool track_btn_en,
        const TrackSettings &track, const QString &logfile, const int logbufsize)
: QDialog(pp), m_devctrls(0), m_rotation(90), m_colortrack_en(track_en)
{
    setupUi(this);

    // label the color tracking button appropriately
    if (m_colortrack_en)
        btnToggleTracking->setText("Disable Tracking");
    else
        btnToggleTracking->setText("Enable Tracking");

    //btnToggleTracking->setEnabled(track_btn_en);

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
    /*
    sbR->setValue(track.color.red());
    sbG->setValue(track.color.green());
    sbB->setValue(track.color.blue());
    sbHt->setValue(track.ht);
    sbSt->setValue(track.st);
    sbFt->setValue(track.ft);
    
    //Set Initial Tracking FPS
    sbTrackingFps->setValue(10);
    */
        
    //TODO: Set LogFile Value       
    editLogFileName->setText(logfile);
    sbLogBuffer->setValue(logbufsize);
}

// -----------------------------------------------------------------------------
SettingsDialog::~SettingsDialog()
{
}

// -----------------------------------------------------------------------------
void SettingsDialog::onDeviceControlUpdated(const QString &name,
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
        cb->setCheckState(current_value ? Qt::Checked : Qt::Unchecked);
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
        for(int i = 0; i <= maximum; i++){
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
void SettingsDialog::onDeviceMenuUpdated(const QString &name, int id, int index)
{
    if (!m_id_to_dev.contains(id))
    {
        Logger::fail("onDeviceMenuUpdated: device control id doesn't exist\n");
        return;
    }

    QComboBox *cb = (QComboBox *)m_id_to_dev.value(id);
    cb->setItemText(index, name);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onTrimSettingsUpdated(int yaw, int pitch, int roll, int thro)
{
    slideYawTrim->setSliderPosition(yaw);
    slidePitchTrim->setSliderPosition(pitch);
    slideRollTrim->setSliderPosition(roll);
    slideThrottleTrim->setSliderPosition(thro);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onFilterSettingsUpdated(int imu, int alt, int aux, int batt)
{
    slideOrientationFilter->setSliderPosition(imu);
    slideAltitudeFilter->setSliderPosition(alt);
    slideAuxiliaryFilter->setSliderPosition(aux);
    slideBatteryFilter->setSliderPosition(batt);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onColorValuesUpdated(TrackSettings track)
{
    sbR->setValue(track.color.red());
    sbG->setValue(track.color.green());
    sbB->setValue(track.color.blue());
    sbHt->setValue(track.ht);
    sbSt->setValue(track.st);
    sbFt->setValue(track.ft);
    
    //Set Initial Tracking FPS
    sbTrackingFps->setValue(track.fps);
    
    onUpdateColorTrackEnable(track.enabled); 
    
   //Logger::info(tr("SETTINGS: Track Enabled %1\n").arg(track.enabled));
   
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
    emit deviceControlChanged(id, (state == Qt::Checked) ? 1 : 0);
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
    emit deviceControlChanged(id, value);
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
    emit deviceControlChanged(id, index);
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
    emit trackSettingsChanged(sbR->value(), sbG->value(), sbB->value(),
            sbHt->value(), sbSt->value(), sbFt->value(),sbTrackingFps->value());

    emit logSettingsChanged(editLogFileName->text(), tr("telemetry.log"), sbLogBuffer->value());
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
void SettingsDialog::onColorTrackingClicked()
{
    if(btnToggleTracking->text() == QString("Disable Tracking"))
    {
        emit updateColorTrackEnable(0);
    } else {
        emit updateColorTrackEnable(1);
    }   
}

// -----------------------------------------------------------------------------
void SettingsDialog::onUpdateColorTrackEnable(int enabled)
{
    m_colortrack_en = enabled; 
    if (enabled)
    {
        btnToggleTracking->setText("Disable Tracking");
    } else {
        btnToggleTracking->setText("Enable Tracking");
    }   
}

// -----------------------------------------------------------------------------
void SettingsDialog::onTrimSliderChanged(int value)
{
    if (sender() == slideThrottleTrim)
        emit trimSettingsChanged(AXIS_ALT, value);
    else if (sender() == slideYawTrim)
        emit trimSettingsChanged(AXIS_YAW, value);
    else if (sender() == slidePitchTrim)
        emit trimSettingsChanged(AXIS_PITCH, value);
    else if (sender() == slideRollTrim)
        emit trimSettingsChanged(AXIS_ROLL, value);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onFilterSliderChanged(int value)
{
    if (sender() == slideOrientationFilter)
        emit filterSettingsChanged(SIGNAL_ORIENTATION, value);
    else if (sender() == slideAltitudeFilter)
        emit filterSettingsChanged(SIGNAL_ALTITUDE, value);
    else if (sender() == slideAuxiliaryFilter)
        emit filterSettingsChanged(SIGNAL_AUXILIARY, value);
    else if (sender() == slideBatteryFilter)
        emit filterSettingsChanged(SIGNAL_BATTERY, value);
}

// -----------------------------------------------------------------------------
void SettingsDialog::onPIDSpinBoxChanged()
{
    int axis    = VCM_AXIS_ALT;
    int signal  = SIGNAL_KP;

    //Determine Signal
    if (sender() == spin_alt_kp     || sender() == spin_yaw_kp ||
        sender() == spin_pitch_kp   || sender() == spin_roll_kp){
        signal = SIGNAL_KP;
    } else if ( sender() == spin_alt_ki     || sender() == spin_yaw_ki ||
                sender() == spin_pitch_ki   || sender() == spin_roll_ki){
        signal = SIGNAL_KI;
    } else if ( sender() == spin_alt_kd     || sender() == spin_yaw_kd ||
                sender() == spin_pitch_kd   || sender() == spin_roll_kd){
        signal = SIGNAL_KD;
    } else if ( sender() == spin_alt_set     || sender() == spin_yaw_set ||
                sender() == spin_pitch_set   || sender() == spin_roll_set){
        signal = SIGNAL_SP;
    }
    
    //Determine Axis
    if (    sender() == spin_alt_kp || 
            sender() == spin_alt_ki ||
            sender() == spin_alt_kd ||
            sender() == spin_alt_set )
    {
        axis    = VCM_AXIS_ALT;
    } else if ( sender() == spin_yaw_kp || 
                sender() == spin_yaw_ki ||
                sender() == spin_yaw_kd ||
                sender() == spin_yaw_set )
    {
        axis    = VCM_AXIS_YAW;
    } else if ( sender() == spin_pitch_kp || 
                sender() == spin_pitch_ki ||
                sender() == spin_pitch_kd ||
                sender() == spin_pitch_set )
    {
        axis    = VCM_AXIS_PITCH;
    } else if ( sender() == spin_roll_kp || 
                sender() == spin_roll_ki ||
                sender() == spin_roll_kd ||
                sender() == spin_roll_set )
    {
        axis    = VCM_AXIS_ROLL;
    }
    
    emit pidSettingsChanged(axis, signal, ((QDoubleSpinBox*)sender())->value());
}

// -----------------------------------------------------------------------------
void SettingsDialog::onPIDSettingsUpdated(int axis, float p, float i, float d, float set)
{   
    switch (axis) {
        case VCM_AXIS_YAW:
            // update double spin boxes
            spin_yaw_kp->setValue(p);
            spin_yaw_ki->setValue(i);
            spin_yaw_kd->setValue(d);
            spin_yaw_set->setValue(set);
            break;
        case VCM_AXIS_PITCH:
            // update double spin boxes
            spin_pitch_kp->setValue(p);
            spin_pitch_ki->setValue(i);
            spin_pitch_kd->setValue(d);
            spin_pitch_set->setValue(set);
            break;
        case VCM_AXIS_ROLL:
            // update double spin boxes
            spin_roll_kp->setValue(p);
            spin_roll_ki->setValue(i);
            spin_roll_kd->setValue(d);
            spin_roll_set->setValue(set);
            break;
        case VCM_AXIS_ALT:
            // update double spin boxes
            spin_alt_kp->setValue(p);
            spin_alt_ki->setValue(i);
            spin_alt_kd->setValue(d);
            spin_alt_set->setValue(set);
            break;
        default:
            Logger::fail(
                tr("onPIDSettingsUpdated: invalid axis: %1\n").arg(axis));          
    }
}

// -----------------------------------------------------------------------------
void SettingsDialog::onRotationIndexChanged(int index)
{
    switch (index)
    {
    case 0: m_rotation = 0; break;
    case 1: m_rotation = 90; break;
    case 2: m_rotation = 180; break;
    case 3: m_rotation = 270; break;
    }
    emit videoRotationChanged(m_rotation);
}

