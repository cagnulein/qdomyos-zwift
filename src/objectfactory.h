#pragma once

#include "lockscreen/qzlockscreen.h"
#include "lockscreen/qzlockscreenfunctions.h"

/**
 * @brief An object factory that can be used to override the classes of object that are returned
 * for various purposes, notably testing.
 */
class ObjectFactory
{
private:
    ObjectFactory() {}
    ObjectFactory(const ObjectFactory&) {}
public:
    static std::function<QZLockscreen*()> lockscreenFactory;
    static QZLockscreen * createLockscreen();

    static std::function<QZLockscreenFunctions*()> lockscreenFunctionsFactory;
    static QZLockscreenFunctions * createLockscreenFunctions();


};

