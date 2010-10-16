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
    editDevice->setEnabled(NetworkDeviceController::m_takesDevice);
}

ConnectionDialog::~ConnectionDialog()
{
}

void ConnectionDialog::s_cancelButton(){
     //printf("CancelButton\n");

    reject();
}

void ConnectionDialog::s_connectButton(){
    
    int index = 0;
    QString source;
    index = cbType->currentIndex(); 
    switch (index){
    case 0: //Network
    source = QString("network");
    break;
    
    case 1: //Serial
    source = QString("serial");
    break;
    
    case 2: //Simulated
    source = QString("sim");
    break;
    }
    
    
    controller = CreateDeviceController(
                source,
                editDevice->text());
    if (!controller)
    {
        //cerr << "invalid source type '" << source << "' specified\n";
        printf("Controller failed\n");
        //return EXIT_FAILURE;
    }
    if(controller->open()){
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

DeviceController * ConnectionDialog::getDeviceController(){
    return controller;
}
