#include <string>

#include <unordered_set>

class HostCounter
{
    public:
        HostCounter();

        // Announce a host to the HostCounter.
        // if the host is new, it will be added to the list, otherwise we ignore it.
        void notifyHost(const std::string& hostname);

        // return the number of unique hosts found so far
        int getNbOfHosts() const;

    private:
        std::unordered_set<std::string> myHosts;
};
