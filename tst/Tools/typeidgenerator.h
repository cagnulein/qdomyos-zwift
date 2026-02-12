#pragma once

#include "devicetypeid.h"

/**
 * @brief The TypeIdGenerator class provides a unique identifier for types.
 */
class TypeIdGenerator
{
private:
    static int count;
public:

    /**
     * @brief Gets a unique type identifier for the specified type.
     * This identifier is generated at runtime not from the type itself, but from the order in which this function is called on types.
     * @return
     */
    template<class T>
    static DeviceTypeId GetTypeId()
    {
        static const int idCounter = count++;
        return DeviceTypeId(idCounter);
    }
};
