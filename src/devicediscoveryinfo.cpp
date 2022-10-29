#include "devicediscoveryinfo.h"


void devicediscoveryinfo::exclude(deviceType type) {
    this->exclusions.push_back(type);
}

bool devicediscoveryinfo::excludes(deviceType type) const {
    for(auto item : exclusions)
        if(item==type) return true;
    return false;
}

