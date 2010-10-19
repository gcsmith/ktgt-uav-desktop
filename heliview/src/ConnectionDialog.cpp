// -----------------------------------------------------------------------------
// File:    ConnectionDialog.cpp
// Authors: Garrett Smith
// -----------------------------------------------------------------------------

#include "ConnectionDialog.h"
#include "DeviceController.h"
#include "NetworkDeviceController.h"
#include "SerialDeviceController.h"
#include "SimulatedDeviceController.h"
#include <stdio.h>
#include <iostream>
#include <QComboBox>
#include <QString>
#include <QMessageBox>

// -----------------------------------------------------------------------------
ConnectionDialog::ConnectionDialog(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);

    connect(btnCancel, SIGNAL(released()), this, SLOT(onCancelClicked()));
    connect(btnConnect, SIGNAL(released()), this, SLOT(onConnectClicked()));
    connect(cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboChanged(int)));
    
    cbType->setCurrentIndex(0);
    onComboChanged(0);
}

// -----------------------------------------------------------------------------
ConnectionDialog::~ConnectionDialog()
{
}

// -----------------------------------------------------------------------------
void ConnectionDialog::onCancelClicked()
{
    reject();
}

// -----------------------------------------------------------------------------
void ConnectionDialog::onConnectClicked()
{
    QString source = cbType->currentText().toLower();
    QString device = editDevice->text();

    emit requestConnection(source, device);
    accept();
}

// -----------------------------------------------------------------------------
void ConnectionDialog::onComboChanged(int index)
{
    QString text = cbType->currentText().toLower();
    if (text == "network")
    {
        lblDescription->setText("Connect to a device over a network. "
                "Device string must be in the form address:port.\n\n"
                "Example:\n    192.168.1.101:8090");
        editDevice->setEnabled(true);
    }
    else if (text == "serial")
    {
        lblDescription->setText("Connect to a serial device. This must be the "
                "serial port of either a 6DOF or 9DOF razor IMU.\n\n"
                "Example (Windows):\n    COM1\n"
                "Example (Unix):\n    /dev/ttyUSB1\n");
        editDevice->setEnabled(true);
    }
    else if (text == "simulated")
    {
        lblDescription->setText("Connect to a simulated device. This is for "
                "testing and demonstration purposes.");
        editDevice->setEnabled(false);
    }
}

