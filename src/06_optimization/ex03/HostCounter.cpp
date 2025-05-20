#include "HostCounter.h"

HostCounter::HostCounter() = default;

void HostCounter::notifyHost(const std::string& hostname)
{
    myHosts.insert(hostname); // insert is safe, does nothing if already present
}

int HostCounter::getNbOfHosts() const
{
    return myHosts.size();
}
