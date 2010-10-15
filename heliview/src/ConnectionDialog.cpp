#include "ConnectionDialog.h"
#include "NetworkDeviceController.h"
#include "SerialDeviceController.h"
#include "SimulatedDeviceController.h"
#include <stdio.h>
#include <iostream>
#include <QComboBox>
ConnectionDialog::ConnectionDialog(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);
    connect(btnCancel, SIGNAL(released()), this, SLOT(s_cancelButton()));
    connect(btnConnect, SIGNAL(released()), this, SLOT(s_connectButton()));
    connect(cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(s_cbChange(int)));
    
    //TODO Remove functions and just connect slots to signals
    
    //Set Default description
    lblDescription->setText(QString(NetworkDeviceController::m_description));
    editDevice->setReadOnly(!NetworkDeviceController::m_takesDevice);
}

ConnectionDialog::~ConnectionDialog()
{
}

void ConnectionDialog::s_cancelButton(){
     //printf("CancelButton\n");

    reject();
}

void ConnectionDialog::s_connectButton(){
    //printf("ConnectButton\n");

    accept();
}
void ConnectionDialog::s_cbChange(int index){
    printf("Combo box selection changed - index: %d\n", index);
    std::cout << qPrintable(cbType->itemText(index)) << std::endl;

    switch (index){
    case 0:
    lblDescription->setText(QString(NetworkDeviceController::m_description));
    editDevice->setReadOnly(!NetworkDeviceController::m_takesDevice);
    break;
    
    case 1:
    lblDescription->setText(QString(SerialDeviceController::m_description));
    editDevice->setReadOnly(!SerialDeviceController::m_takesDevice);
    break;
    
    case 2:
    lblDescription->setText(QString(SimulatedDeviceController::m_description));
    editDevice->setReadOnly(!SimulatedDeviceController::m_takesDevice);
    break;
    }
    
}
