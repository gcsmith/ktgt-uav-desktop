// -----------------------------------------------------------------------------
// File:    HeliView.cpp
// Authors: Garrett Smith, Tim Miller, Kevin Macksamie, Tyler Thierolf
//
// Entry point for HeliView application.
// -----------------------------------------------------------------------------

#include <iostream>
#include <QMessageBox>
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
    bool disable_virtual_view = false;

    po::options_description desc("Program options");
    desc.add_options()
        S_ARG("source,s", "select data source (network|serial|sim)")
        S_ARG("device,d", "specify device for network or serial communication")
        S_ARG("log,l",    "specify log file path")
        N_ARG("help,h",   "produce this help message")
        N_ARG("novirtual","disable the virtual view pane");

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

        disable_virtual_view = !vm.count("novirtual");
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

    try
    {
        DeviceController *controller = CreateDeviceController(
                QString::fromStdString(source),
                QString::fromStdString(device));
        if (!controller)
        {
            cerr << "invalid source type '" << source << "' specified\n";
            return EXIT_FAILURE;
        }

        // create and show the interface
        ApplicationFrame frame(controller, disable_virtual_view);
        if (0 != logfile.length())
        {
            frame.openLogFile(QString::fromStdString(logfile));
            frame.enableLogging(true);
        }

        frame.show();
        return app.exec();
    }
    catch (exception &e)
    {
        QString msg(e.what());
        msg.append("\n===================================================\n");
        msg.append("Make sure 'cfg' and 'media' exist in the ");
        msg.append("directory as the HeliView executable.");

        QMessageBox mb(QMessageBox::Critical, "Fatal Error", msg);
        mb.exec();
        return EXIT_FAILURE;
    }
}

// -----------------------------------------------------------------------------
#if defined(_MSC_VER) && defined(_WIN32)
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
    QStringList strlist = QString(lpCmdLine).split(" ");

    vector<string> strvec;
    strvec.push_back("heliview");
    for (int i = 0; i < strlist.size(); ++i)
        strvec.push_back(strlist[i].toStdString());

    vector<char *> ptrvec;
    for (size_t i = 0; i < strvec.size(); ++i)
        ptrvec.push_back(&strvec[i][0]);

    return main(ptrvec.size(), &ptrvec[0]);
}
#endif

