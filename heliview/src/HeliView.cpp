// -----------------------------------------------------------------------------
// File:    HeliView.cpp
// Authors: Garrett Smith, Tim Miller, Kevin Macksamie, Tyler Thierolf
//
// Entry point for HeliView application.
// -----------------------------------------------------------------------------

#include <iostream>
#include <QApplication>
#include <QDebug>
#include "ApplicationFrame.h"
#include "DeviceController.h"
#include "CommandLine.h"

using namespace std;
namespace po = boost::program_options;

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    string source("network"), logfile("heliview.log"), device;

    bool show_usage = false;
    po::options_description desc("Program options");
    desc.add_options()
        S_ARG("source,s", "select data source (network|serial|sim)")
        S_ARG("device,d", "specify device for network or serial communication")
        S_ARG("log,l",    "specify log file path")
        N_ARG("help,h",   "produce this help message");

    try
    {
        // parse the command line arguments
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        optional_arg(vm, "source", source);
        optional_arg(vm, "device", device);
        optional_arg(vm, "log", logfile);

        show_usage = !!vm.count("help");
    }
    catch (exception &e)
    {
        // some critical error occurred - print error and exit
        cerr << "command line error " << "(" << e.what() << ")\n";
        show_usage = true;
    }

    if (show_usage)
    {
        // print the usage message
        cerr << "usage: HeliView [options]\n\n" << desc << endl;
        return EXIT_FAILURE;
    }

    DeviceController *controller = CreateDeviceController(source);
    if (!controller)
    {
        cerr << "invalid source type '" << source << "' specified\n";
        return EXIT_FAILURE;
    }

    // attempt to open the specified device
    if (!controller->open(QString::fromStdString(device)))
    {
        cerr << "failed to open device '" << device << "'\n";
        return EXIT_FAILURE;
    }

    // create and show the interface
    ApplicationFrame frame(controller);
    if (0 != logfile.length())
    {
        frame.openLogFile(QString::fromStdString(logfile));
        frame.enableLogging(true);
    }

    frame.show();
    return app.exec();
}

