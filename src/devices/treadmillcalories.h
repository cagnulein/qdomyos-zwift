#ifndef TREADMILLCALORIES_H
#define TREADMILLCALORIES_H

#include <cmath>

// Treadmill calories from a walking/running energy model, kept apart from the power-based value.
// No Qt dependency, so the accumulation can be tested with synthetic ticks. treadmill calls
// tick() from update_metrics() once per update while the session is not paused.
//
// One running total is fed by whichever source is in use at each tick: the model's kcal for the
// tick, or the increase of the power-based (or console) total. Switching source mid-workout (a
// power sensor that connects, the setting toggled) therefore continues the total instead of
// jumping to the other source's value. While the model has never been used in the session,
// shown() is the power-based total itself, so with the setting off nothing changes.
class TreadmillCalories {
  public:
    // Energy model (sources in treadmill.cpp). Returns kcal/min including resting metabolism;
    // speed km/h, incline %, weight kg, height cm, restingKcalPerDay from Mifflin-St Jeor.
    // A stopped belt gives the resting rate. *running tells which gait was used.
    static double walkRunTransitionKmh(double heightCm) {
        if (heightCm < 100.0 || heightCm > 250.0)
            heightCm = 175.0;
        const double legLengthM = 0.5 * heightCm / 100.0;
        return std::sqrt(0.5 * 9.81 * legLengthM) * 3.6;
    }

    static double kcalPerMinute(double speedKmh, double inclinePct, double weightKg, double heightCm,
                                double restingKcalPerDay, bool *running = nullptr) {
        const double kcalPerMlO2 = 20.1 / 4184.0;
        const double restingKcalPerMin = restingKcalPerDay > 0 ? restingKcalPerDay / 1440.0 : 0;
        if (running)
            *running = false;
        if (speedKmh <= 0 || weightKg <= 0)
            return restingKcalPerMin;

        const bool run = speedKmh >= walkRunTransitionKmh(heightCm);
        if (running)
            *running = run;
        double netMlKgMin;
        if (run) {
            const double S = speedKmh * 1000.0 / 60.0;
            const double G = inclinePct > 0 ? inclinePct / 100.0 : 0;
            netMlKgMin = 0.2 * S + 0.9 * S * G;
        } else {
            const double s = speedKmh / 3.6;
            if (inclinePct >= 0)
                netMlKgMin = 0.32 * inclinePct + 3.28 + (1.0 + 0.19 * inclinePct) * 2.66 * s * s;
            else
                netMlKgMin = 0.73 * (3.28 + 2.66 * s * s);
        }
        return restingKcalPerMin + netMlKgMin * weightKg * kcalPerMlO2;
    }

    struct Tick {
        double dtSec = 0;          // time since the previous update
        bool clockRunning = false; // the session clock advanced (belt moving, or continuous_moving)
        double speedKmh = 0;
        double inclinePct = 0;
        double weightKg = 0;
        double heightCm = 0;
        double restingKcalPerDay = 0;
        bool useModel = false;     // setting on, no console calories, no power measurement
        double legacyKcal = 0;     // the power-based (or console) total of the session now
    };

    // Start of a session; legacyKcal is the power-based total at that moment.
    void reset(double legacyKcal = 0) {
        m_total = 0;
        m_legacyBase = legacyKcal;
        m_lastLegacy = legacyKcal;
        m_modelUsed = false;
        m_lastPerMin = 0;
        m_lastRunning = false;
    }

    // Returns the kcal added to the total by this tick.
    double tick(const Tick &t) {
        double legacyDelta = t.legacyKcal - m_lastLegacy;
        m_lastLegacy = t.legacyKcal;
        if (!std::isfinite(legacyDelta) || legacyDelta < 0)
            legacyDelta = 0; // a source that reset or sent garbage does not take calories away

        double add = legacyDelta;
        if (t.useModel) {
            m_modelUsed = true;
            add = 0;
            m_lastPerMin = kcalPerMinute(t.speedKmh, t.inclinePct, t.weightKg, t.heightCm, t.restingKcalPerDay,
                                         &m_lastRunning);
            // resting calories too while the clock runs with the belt stopped, so that active
            // calories (total minus resting) do not go down
            if (t.clockRunning && t.dtSec > 0 && std::isfinite(t.dtSec))
                add = m_lastPerMin * t.dtSec / 60.0;
        }
        m_total += add;
        return add;
    }

    bool modelUsed() const { return m_modelUsed; }
    double total() const { return m_total; }
    // The session total to show: the power-based total itself while the model was never used.
    double shown(double legacyKcal) const { return m_modelUsed ? m_total : legacyKcal - m_legacyBase; }
    double lastKcalPerMinute() const { return m_lastPerMin; }
    bool lastRunning() const { return m_lastRunning; }

  private:
    double m_total = 0;
    double m_legacyBase = 0;
    double m_lastLegacy = 0;
    bool m_modelUsed = false;
    double m_lastPerMin = 0;
    bool m_lastRunning = false;
};

#endif // TREADMILLCALORIES_H
