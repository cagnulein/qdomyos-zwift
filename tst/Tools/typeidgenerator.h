#pragma once

#include "devicetypeid.h"

class TypeIdGenerator
{
private:
    static int count;
public:
    template<class T>
    static DeviceTypeId GetTypeId()
    {
        static const int idCounter = count++;
        return DeviceTypeId(idCounter);
    }
};
