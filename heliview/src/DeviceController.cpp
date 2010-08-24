#include <string>
#include "NetworkDeviceController.h"
#include "SerialDeviceController.h"
#include "SimulatedDeviceController.h"

using namespace std;

DeviceController *CreateDeviceController(const string &name)
{
    if (name == "network")
        return new NetworkDeviceController;
    else if (name == "serial")
        return new SerialDeviceController;
    else if (name == "sim")
        return new SimulatedDeviceController;
    else
        return NULL;
}

