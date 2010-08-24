// -----------------------------------------------------------------------------
// File:    HeliView.cpp
// Authors: Garrett Smith, Tim Miller, Kevin Macksamie, Tyler Thierolf
//
// Entry point for HeliView application.
// -----------------------------------------------------------------------------

#include <iostream>
#include <boost/program_options.hpp>
#include <QApplication>
#include <QDebug>
#include "ApplicationFrame.h"
#include "DeviceController.h"

using namespace std;
namespace po = boost::program_options;

#define S_ARG(name, desc) (name, po::value<string>(), desc)
#define N_ARG(name, desc) (name, desc)

// -----------------------------------------------------------------------------
template <typename T>
void optional_arg(po::variables_map &vm, const string &name, T &var)
{
    if (vm.count(name))
        var = vm[name].as<T>();
}

// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    string source("network"), logfile("heliview.log"), device;

    po::options_description desc("Program options");
    desc.add_options()
        S_ARG("source,s", "select data source (network|serial|sim)")
        S_ARG("device,d", "specify device for network or serial communication")
        S_ARG("log,l",    "specify log file path")
        N_ARG("help,h",   "produce this help message");

    try
    {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        optional_arg(vm, "source", source);
        optional_arg(vm, "device", device);
        optional_arg(vm, "log", logfile);
    }
    catch (exception &e)
    {
        cerr << "command line error\n" << desc;
        return EXIT_FAILURE;
    }

    DeviceController *controller = CreateDeviceController(source);
    if (!controller)
    {
        cerr << "invalid source type '" << source << "' specified\n";
        return EXIT_FAILURE;
    }

    if (!controller->open(QString::fromStdString(device)))
    {
        cerr << "failed to open device '" << device << "'\n";
        return EXIT_FAILURE;
    }

    ApplicationFrame frame(controller);
    if (0 != logfile.length())
    {
        frame.openLogFile(QString::fromStdString(logfile));
        frame.enableLogging(true);
    }

    frame.show();
    return app.exec();
}

