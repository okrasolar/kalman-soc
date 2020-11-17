#pragma once

#include <stdint.h>

/**
 * @brief Calculated battery state of charge (SoC) using a extended kalman filter.
 * Math currently uses floating point arithmetic. Can only record diffreences in soc at a 30 min interval
 * or greater.
 */
class SoCKalman
{
  public:
    SoCKalman();

    /**
     * @brief initial soc is either passed in after being retrieved from local storage,
     *        or is estimated based on starting battery voltage
     *
     * @param batteryEff, batteryVoltage, initialSoC (optional)
     */
    void init(bool isBattery12V, bool isBatteryLithium, uint32_t batteryEff, uint32_t batteryVoltage, uint32_t initialSoC);

    /**
     * @brief return current state of charge
     *
     * @return uint32_t soc
     */
    uint32_t read();

    /**
     * @brief return current battery efficiency
     *
     * @return uint32_t _batteryEff
     */
    uint32_t efficiency();

    /**
     * @brief calculate new soc based on how much power entered/exited the battery in a given
     * window as well as the battery voltage, also recalculate battery efficiency and 
     * reset soc = 100% if battery is in float
     *
     * @param isBatteryInFloat, isBatteryLithium, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity
     */
    void sample(bool isBatteryInFloat, int32_t batteryMilliAmps, uint32_t batteryVoltage, int32_t batteryMilliWatts, uint32_t samplePeriodMilliSec,
        uint32_t batteryCapacity);

  private:
    uint32_t _previousSoC;
    uint32_t _batteryEff;
    float _pval;
    float _qval;
    float _rval;
    float _pPre[9];
    float _pPost[9];
    float _q[9];
    float _a[9];
    float _at[9];
    float _h;
    float _H[3];
    float _Ht[3];
    float _G[3];
    bool _isBattery12V;
    bool _isBatteryLithium;
    uint32_t _millisecondsInFloat = 0;
    uint32_t _floatResetDuration = 600000;  // 10 minutes in milliseconds
    int32_t _x[3] = { 0, 0, 0 };
    uint8_t _n = 3;
    uint8_t _m = 1;
    const uint32_t SOC_SCALED_HUNDRED_PERCENT = 100000;  // 100% charge = 100000
    const uint32_t SOC_SCALED_MAX = 2 * SOC_SCALED_HUNDRED_PERCENT;  // allow soc to track up higher than 100% to gauge efficiency

    /**
     * @brief estimate an initial soc based on battery voltage
     *
     * @param batteryVoltage
     *
     * @return uint32_t soc
     */
    uint32_t calculateInitialSoC(uint32_t batteryVoltage);

    /**
     * @brief project the state of charge ahead one step using a Coulomb counting model
     * 
     * @param isBatteryinFloat, batteryMilliWatts, samplePeridoMilliSec, batteryCapacity
     */
    void f(bool isBatteryInFloat, int32_t batteryMilliWatts, uint32_t samplePeriodMilliSec, uint32_t batteryCapacity);

    /**
     * @brief predict the measurable value (voltage) ahead one step using the newly estimated state of charge
     * 
     * @param isBatteryLithium, batteryMilliAmps
     */
    void h(int32_t batteryMilliAmps);

    void diagonalMatrix(float value, float* result);

    void matMult(float* a, float* b, float* result, uint8_t arows, uint8_t acols, uint8_t bcols);

    void matMultConst(float* a, float b, float* result, uint8_t length);

    void matAdd(float* a, float* b, float* result, uint8_t length);

    void matAccum(float* a, float* b, uint8_t length);

    void transpose(float* a, float* result, uint8_t rows, uint8_t cols);

    void negate(float* a, uint8_t length);

    void updateState(float* a, uint8_t length);

    uint8_t inverse(float* a, float* result);

    uint32_t clamp(uint32_t value, uint32_t min, uint32_t max);

};