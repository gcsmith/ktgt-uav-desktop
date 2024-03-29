// -----------------------------------------------------------------------------
// File:    SerialDeviceController.cpp
// Authors: Garrett Smith
// Created: 08-24-2010
//
// Serial device interface implementation.
// -----------------------------------------------------------------------------

#include <iostream>
#include <QDebug>
#include "Logger.h"
#include "SerialDeviceController.h"
#include "Utility.h"

using namespace std;

const char * SerialDeviceController::m_description = "Serial description";
const bool SerialDeviceController::m_takesDevice = true;

// -----------------------------------------------------------------------------
SerialDeviceController::SerialDeviceController(const QString &device)
: m_device(device), m_serial(NULL), m_buffer(1024, 0), m_offset(0)
{
}

// -----------------------------------------------------------------------------
SerialDeviceController::~SerialDeviceController()
{
    close();
}

// -----------------------------------------------------------------------------
bool SerialDeviceController::open()
{
    // make sure to close if there's a device currently open
    close();
    qDebug() << "creating serial device" << m_device;
    Logger::info(QString("SDC: creating serial device\n"));

    m_serial = new QextSerialPort(m_device, QextSerialPort::EventDriven);
    m_serial->setBaudRate(BAUD57600);
    m_serial->setDataBits(DATA_8);
    m_serial->setParity(PAR_NONE);
    m_serial->setStopBits(STOP_1);
    m_serial->setFlowControl(FLOW_OFF);

    connect(m_serial, SIGNAL(readyRead()), this, SLOT(onSerialDataReady()));

    if (!m_serial->open(QIODevice::ReadOnly))
        return false;

    onSerialDataReady();
    return true;
}

// -----------------------------------------------------------------------------
void SerialDeviceController::close()
{
    if (m_serial)
    {
        m_serial->close();
        SafeDelete(m_serial);
    }
}

// -----------------------------------------------------------------------------
void SerialDeviceController::onSerialDataReady()
{
    int bytes_avail = m_serial->bytesAvailable();
    if (!bytes_avail)
        return;
    
    QByteArray data;
    data.resize(bytes_avail);
    m_serial->read(data.data(), data.size());

    for (int i = 0; i < bytes_avail; i++)
    {
        switch (data[i])
        {
        case '!':
            m_offset = 0;
            m_validLine = true;
            break;
        case '\n':
            m_buffer[m_offset - 1] = '\0';
            if (m_validLine)
            {
                // only process complete lines
                processSingleLine(&m_buffer[0]);
                m_validLine = false;
            }
            m_offset = 0;
            break;
        default:
            m_buffer[m_offset] = data[i];
            m_offset++;
            break;
        }
    }
}

// -----------------------------------------------------------------------------
void SerialDeviceController::processSingleLine(const string &line)
{
    if (line[0] != 'A' || line[1] != 'N' || line[2] != 'G' || line[3] != ':')
    {
        cerr << "encountered invalid line (no prefix)\n";
        return;
    }

    QString str = QString::fromStdString(line.substr(4));
    QStringList ssplit = str.split(",", QString::KeepEmptyParts);
    if (ssplit.size() != 3)
    {
        cerr << "encountered invalid line (!= 3 numbers)\n";
        return;
    }

    float x = ssplit[0].toFloat();
    float y = ssplit[1].toFloat();
    float z = ssplit[2].toFloat();

    emit telemetryReady(z, y, x, 0, 200, 100, 1500, 0);
    Logger::telemetry(tr("%1 %2 %3 %4 %5 %6 %7 %8\n").arg(z).arg(y).arg(x)
                     .arg(0).arg(200).arg(100).arg(1500).arg(0));
}

