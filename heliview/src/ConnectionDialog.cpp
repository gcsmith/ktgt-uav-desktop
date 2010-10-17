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
ConnectionDialog::ConnectionDialog(QWidget *parent, DeviceController *controller)
: QDialog(parent), m_controller(controller)
{
    setupUi(this);
    
    connect(btnCancel, SIGNAL(released()), this, SLOT(s_cancelButton()));
    connect(btnConnect, SIGNAL(released()), this, SLOT(s_connectButton()));
    connect(cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(s_cbChange(int)));
    
    
    //Set Default description
    editDevice->setText(m_controller->device());
    
    
    if(QString::compare(QString("network"),m_controller->controllerType()) == 0){
        cbType->setCurrentIndex(0);
        lblDescription->setText(QString(NetworkDeviceController::m_description));
    } else if(QString::compare(QString("serial"),m_controller->controllerType()) == 0){
        cbType->setCurrentIndex(1);
        lblDescription->setText(QString(SerialDeviceController::m_description));
    } else if(QString::compare(QString("sim"),m_controller->controllerType()) == 0){
        cbType->setCurrentIndex(2);
        lblDescription->setText(QString(SimulatedDeviceController::m_description));
    }
    
       
}

ConnectionDialog::~ConnectionDialog()
{
}

void ConnectionDialog::s_cancelButton(){
     //printf("CancelButton\n");

    reject();
}

void ConnectionDialog::s_connectButton(){
    //if(!m_controller
    
    m_controller->close();
    
    delete m_controller;
    switch (cbType->currentIndex()){
    case 0: //Network
    m_controller = new NetworkDeviceController(editDevice->text());
    break;
    
    case 1: //Serial
    m_controller = new SerialDeviceController(editDevice->text());
    break;
    
    case 2: //Simulated
    m_controller = new SimulatedDeviceController(editDevice->text());
    break;
    }

    if(m_controller->open()){
        accept();
    } else {
        QMessageBox mb(QMessageBox::Warning,
                   "Connection Failure",
                   "Connection Failed to Open - Please check your device string");
        mb.exec();    
    } 
    
}

void ConnectionDialog::s_cbChange(int index){
    printf("Combo box selection changed - index: %d\n", index);
    std::cout << qPrintable(cbType->itemText(index)) << std::endl;
    
    switch (index){
    case 0: //Network
    lblDescription->setText(QString(NetworkDeviceController::m_description));
    editDevice->setEnabled(NetworkDeviceController::m_takesDevice);
    break;
    
    case 1: //Serial
    lblDescription->setText(QString(SerialDeviceController::m_description));
    editDevice->setEnabled(SerialDeviceController::m_takesDevice);
    break;
    
    case 2: //Simulated
    lblDescription->setText(QString(SimulatedDeviceController::m_description));
    editDevice->setEnabled(SimulatedDeviceController::m_takesDevice);
    break;
    }
    
}
