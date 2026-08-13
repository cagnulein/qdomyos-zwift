#pragma once

#include <gtest/gtest.h>

#include <vector>

#include "devices/ftmsbike/servicesubscriptionplan.h"

namespace {

/** A service that has resolved and can be subscribed. */
ServiceView discovered(uint64_t id) { return ServiceView{id, ServiceDiscoveryState::Discovered}; }
/** A service that has resolved to nothing. */
ServiceView invalid(uint64_t id) { return ServiceView{id, ServiceDiscoveryState::Invalid}; }
/** Discovery requested, still in flight. */
ServiceView discovering(uint64_t id) { return ServiceView{id, ServiceDiscoveryState::Discovering}; }
/** Discovery not requested yet. */
ServiceView required(uint64_t id) { return ServiceView{id, ServiceDiscoveryState::Required}; }

/**
 * Runs the plan the way ftmsbike::subscribeToServices() does - consult the gate, subscribe
 * whatever is pending, record it - and counts how many times each service was subscribed.
 */
struct planDriver {
    ServiceSubscriptionPlan plan;
    std::vector<uint64_t> subscribeCalls;

    /** One firing of stateChanged() against the given list. Returns true if the gate opened. */
    bool present(const std::vector<ServiceView> &services) {
        if (!plan.allResolved(services)) {
            return false;
        }
        for (uint64_t id : plan.pending(services)) {
            subscribeCalls.push_back(id);
            plan.markSubscribed(id);
        }
        return true;
    }

    int countOf(uint64_t id) const {
        int n = 0;
        for (uint64_t seen : subscribeCalls) {
            if (seen == id) {
                n++;
            }
        }
        return n;
    }
};

} // namespace

TEST(ServiceSubscriptionPlanTest, PartialListGrowthSubscribesEveryServiceExactlyOnce) {
    // The bug that cost two evenings. On Qt's Win32 backend discoverDetails() resolves
    // synchronously, so the slot fires once per service against a list holding only the services
    // built so far: one, then two, then all six. A single "already did the pass" flag latches on
    // the first firing and every later service is skipped forever - the bike then reports nothing
    // but battery. Per-service bookkeeping is the only thing that survives this input.
    planDriver d;

    d.present({discovered(1)});
    d.present({discovered(1), discovered(2)});
    d.present({discovered(1), discovered(2), discovered(3), discovered(4), discovered(5), discovered(6)});

    for (uint64_t id = 1; id <= 6; id++) {
        EXPECT_EQ(1, d.countOf(id)) << "service " << id << " was not subscribed exactly once";
    }
    EXPECT_EQ(6, d.plan.subscribedCount());
}

TEST(ServiceSubscriptionPlanTest, RepeatFiringsOfACompleteListSubscribeOnlyOnce) {
    // The original battery-only bug. stateChanged() is connected to every service, so it fires
    // once per service even after the last one has resolved, and the pass used to run in full each
    // time - four CCCD writes per session on the control point. A reconnection rejects the
    // repeats, indications are never enabled, and the session is dead.
    planDriver d;
    const std::vector<ServiceView> complete = {discovered(10), discovered(20), discovered(30)};

    for (int i = 0; i < 5; i++) {
        EXPECT_TRUE(d.present(complete));
    }

    EXPECT_EQ(3u, d.subscribeCalls.size()) << "the pass ran again on a later firing";
    EXPECT_EQ(1, d.countOf(10));
    EXPECT_EQ(1, d.countOf(20));
    EXPECT_EQ(1, d.countOf(30));
}

TEST(ServiceSubscriptionPlanTest, GateWaitsForEveryServiceToSettle) {
    ServiceSubscriptionPlan plan;

    EXPECT_FALSE(plan.allResolved({discovered(1), required(2)})) << "opened with a service still to be discovered";
    EXPECT_FALSE(plan.allResolved({discovered(1), discovering(2)})) << "opened with a discovery in flight";
    EXPECT_FALSE(plan.allResolved({required(1)}));
    EXPECT_FALSE(plan.allResolved({discovering(1)}));

    EXPECT_TRUE(plan.allResolved({discovered(1), discovered(2)}));
    EXPECT_TRUE(plan.allResolved({invalid(1), invalid(2)}));
    EXPECT_TRUE(plan.allResolved({discovered(1), invalid(2)})) << "a mix of discovered and invalid is settled";
}

TEST(ServiceSubscriptionPlanTest, InvalidServicesAreResolvedButNeverSubscribed) {
    // Stale service objects from a previous connection go InvalidService. They must not hold the
    // gate shut - the connection would never proceed - and there is nothing on them to subscribe.
    planDriver d;

    ASSERT_TRUE(d.present({discovered(1), invalid(2), discovered(3)}));

    EXPECT_EQ(1, d.countOf(1));
    EXPECT_EQ(0, d.countOf(2)) << "subscribed to an invalid service";
    EXPECT_EQ(1, d.countOf(3));
    EXPECT_EQ(2, d.plan.subscribedCount());
}

TEST(ServiceSubscriptionPlanTest, ResetMakesEverythingPendingAgain) {
    planDriver d;
    const std::vector<ServiceView> services = {discovered(1), discovered(2)};

    ASSERT_TRUE(d.present(services));
    ASSERT_EQ(2u, d.subscribeCalls.size());

    d.plan.reset();
    EXPECT_EQ(0, d.plan.subscribedCount());

    d.present(services);
    EXPECT_EQ(2, d.countOf(1)) << "a reconnection must subscribe again";
    EXPECT_EQ(2, d.countOf(2));
}

TEST(ServiceSubscriptionPlanTest, AnIdReusedAfterResetIsTreatedAsNew) {
    // The ids are recycled pointers: the allocator hands the same address back for a service
    // object built on the next connection. Carrying the record across the reset would read the new
    // service as already subscribed and silently skip it.
    planDriver d;

    ASSERT_TRUE(d.present({discovered(0xBEEF)}));
    ASSERT_EQ(1, d.countOf(0xBEEF));

    d.plan.reset();
    d.present({discovered(0xBEEF)});

    EXPECT_EQ(2, d.countOf(0xBEEF)) << "the recycled pointer was mistaken for the old service";
}

TEST(ServiceSubscriptionPlanTest, AnEmptyListDoesNotOpenTheGate) {
    // "Nothing to wait for" must not read as "ready to act on everything". Opening here is how the
    // Win32 bug expressed itself: a gate that is trivially true of a list that is not yet built.
    ServiceSubscriptionPlan plan;

    EXPECT_FALSE(plan.allResolved({}));
    EXPECT_TRUE(plan.pending({}).empty());
}
