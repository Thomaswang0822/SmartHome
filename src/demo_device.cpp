// Placeholder content

#include "device.h"

#include <iostream>

void DemoDevice::Operate()
{
    std::cout << "I am a " << this->GetName() << " and I do NOTHING!" << std::endl;
}