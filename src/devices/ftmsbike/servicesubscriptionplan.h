#ifndef SERVICESUBSCRIPTIONPLAN_H
#define SERVICESUBSCRIPTIONPLAN_H

// Deliberately no Qt include. The whole point of this header is that the decision it holds can be
// exercised without a Bluetooth stack, a Qt event loop, or a device, so it takes nothing from Qt -
// not even <QtGlobal> for an integer typedef.
#include <algorithm>
#include <cstdint>
#include <vector>

/**
 * @brief The discovery state of one GATT service, as this decision cares about it.
 *
 * A deliberate narrowing of QLowEnergyService::ServiceState: the only distinction that matters
 * here is "settled" versus "still moving", plus whether a settled service is usable.
 */
enum class ServiceDiscoveryState {
    Required,    ///< discovery not requested yet
    Discovering, ///< requested, still in flight
    Discovered,  ///< resolved and usable
    Invalid      ///< resolved, and there is nothing there to subscribe to
};

/**
 * @brief One service, reduced to the two facts the plan needs.
 *
 * The id is opaque and the caller owns its meaning - ftmsbike uses the QLowEnergyService pointer.
 * Keeping it opaque is what lets this whole decision be tested without a Bluetooth stack.
 */
struct ServiceView {
    // A plain aggregate, with no default member initializers on purpose: the test
    // project builds at -std=gnu++11, where a struct carrying them is not an
    // aggregate and ServiceView{id, state} does not compile. Keeping it C++11 is
    // part of "builds anywhere", so construct these with both members supplied.
    uint64_t id;
    ServiceDiscoveryState state;
};

/**
 * @brief Which services still need subscribing, and whether it is time to subscribe any of them.
 *
 * This exists because the decision it encapsulates has been wrong twice, in opposite directions,
 * and neither error was reachable from a test.
 *
 * The first: the subscription pass ran in full every time a service changed state, rewriting
 * CCCDs that had already been written - on one bike, the control point's four times per session.
 * A fresh connection tolerates the repeats. A reconnection does not: the device rejects them,
 * indications are never enabled, the control point never acknowledges REQUEST_CONTROL, and the
 * bike reports nothing but battery.
 *
 * The second, fixing the first: a single "already did the pass" flag. That is correct only if the
 * pass sees the complete list, and on Qt's Win32 backend it did not - service discovery resolved
 * synchronously as each service object was built, so the first firing arrived with a list of one,
 * subscribed it, set the flag, and every other service was skipped forever. Correct on Android,
 * catastrophic on Windows, and two evenings to find.
 *
 * So the bookkeeping is per service, and the gate is separate from it. Both are pure functions of
 * the caller's inputs, and no Qt object, clock or radio is involved in either.
 */
class ServiceSubscriptionPlan {
  public:
    /**
     * @brief Is every service settled, so that a subscription pass would see the whole list?
     *
     * An empty list is deliberately *not* resolved. "Nothing to wait for" and "ready to act on
     * everything" look identical to a caller that only asks whether the gate is open, and opening
     * it onto an empty list is how the Win32 bug expressed itself in the first place.
     */
    bool allResolved(const std::vector<ServiceView> &services) const {
        if (services.empty()) {
            return false;
        }
        return std::all_of(services.begin(), services.end(), [](const ServiceView &s) {
            return s.state == ServiceDiscoveryState::Discovered || s.state == ServiceDiscoveryState::Invalid;
        });
    }

    /**
     * @brief The services that are ready to subscribe and have not been subscribed yet.
     *
     * Invalid services never appear here: they are resolved as far as the gate is concerned, but
     * there is nothing on them to subscribe to.
     */
    std::vector<uint64_t> pending(const std::vector<ServiceView> &services) const {
        std::vector<uint64_t> out;
        for (const ServiceView &s : services) {
            if (s.state == ServiceDiscoveryState::Discovered && !isSubscribed(s.id)) {
                out.push_back(s.id);
            }
        }
        return out;
    }

    bool isSubscribed(uint64_t id) const {
        return std::find(m_subscribed.begin(), m_subscribed.end(), id) != m_subscribed.end();
    }

    void markSubscribed(uint64_t id) {
        if (!isSubscribed(id)) {
            m_subscribed.push_back(id);
        }
    }

    /**
     * @brief Forget every subscription, as when a new set of service objects is about to be built.
     *
     * Ids are only meaningful between resets. The caller's ids are recycled pointers, so an id seen
     * after a reset may address a different service than the same id did before it - which is
     * exactly why the record cannot outlive the objects it stands for.
     */
    void reset() { m_subscribed.clear(); }

    int subscribedCount() const { return static_cast<int>(m_subscribed.size()); }

  private:
    std::vector<uint64_t> m_subscribed;
};

#endif // SERVICESUBSCRIPTIONPLAN_H
