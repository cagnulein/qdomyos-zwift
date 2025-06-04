#include "kalmanfilter.h"

KalmanFilter::KalmanFilter(double measure_error, double estimate_error, double process_noise_q, double initial_Value) {
    setMeasurementError(measure_error);
    setEstimateError(estimate_error);
    setProcessNoise(process_noise_q);
    estimate = initial_Value;
}

KalmanFilter::~KalmanFilter() {};

double KalmanFilter::updateEstimate(double measurement) {
    estimate_error += process_noise_q;
    gain = estimate_error / (estimate_error + measure_error);
    estimate += gain * (measurement - estimate);
    estimate_error *= (1 - gain);
    return estimate;
}

void KalmanFilter::setMeasurementError(double measure_err) { measure_error = measure_err; }

void KalmanFilter::setEstimateError(double estimate_err) { estimate_error = estimate_err; }

void KalmanFilter::setProcessNoise(double noise_q) { process_noise_q = noise_q; }

double KalmanFilter::getGain() { return gain; }

double KalmanFilter::getEstimateError() { return estimate_error; }
