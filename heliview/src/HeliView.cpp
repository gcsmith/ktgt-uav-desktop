// -----------------------------------------------------------------------------
// File:    HeliView.cpp
// Authors: Garrett Smith, Tim Miller, Kevin Macksamie, Tyler Thierolf
//
// Entry point for HeliView application.
// -----------------------------------------------------------------------------

#include <QApplication>
#include <QDebug>
#include <unistd.h>
#include "ApplicationFrame.h"

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // parse the command line arguments 
    QString device = "/dev/null";
    QString scheme = "none";
    QString logfile = "";

    char c;
    while (-1 != (c = getopt(argc, argv, "s:d:l:")))
    {
        switch (c)
        {
        case 's':
            scheme = QString(optarg).toLower();
            break;
        case 'd':
            device = optarg;
            break;
        case 'l':
            logfile = optarg;
            break;
        default:
            return 1;
        }
    }
 
    QApplication app(argc, argv);
    ApplicationFrame frame;

    if (scheme == "serial")
    {
        // use the serial port
        if (!frame.openSerialCommunication(device))
            return 1;
    }
    else if (scheme == "sim")
    {
        // generate a simulated sensor input without noise
        frame.attachSimulatedSource(false);
    }
    else if (scheme == "nsim")
    {
        // generate a simulated sensor input with random noise
        frame.attachSimulatedSource(true);
    }
    else if (scheme != "none")
    {
        qDebug() << "invalid scheme" << scheme << "specified";
        return 1;
    }

    if (logfile.length() != 0)
    {
        frame.openLogFile(logfile);
        frame.enableLogging(true);
    }

    frame.show();
    return app.exec();
}

