/*
 * Copyright (c) 2024 Marcel Verpaalen (marcel@verpaalen.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * This emulates a serial port over a network connection.
 * e.g. as created by ser2net or hardware serial to ethernet converters
 *
 */

#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <math.h>

/**
 * @brief A simple Kalman filter for smoothing noisy data.
 */
class KalmanFilter {

  public:
    KalmanFilter(double measure_error, double estimate_error, double process_noise_q = 0, double initial_Value = 0);
    ~KalmanFilter();

    double updateEstimate(double measurement);
    void setMeasurementError(double measure_err);
    void setEstimateError(double estimate_err);
    void setProcessNoise(double noise_q);
    double getGain();
    double getEstimateError();

  private:
    double measure_error;
    double estimate_error;
    double process_noise_q;
    double estimate = 0;
    double gain = 0;
};

#endif
