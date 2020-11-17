#include "SoCKalman.h"

SoCKalman::SoCKalman() :
    _previousSoC(0),
    _batteryEff(0),
    _pval(0.1),
    _qval(0.0001),
    _rval(0.1)

{}

void SoCKalman::init(bool isBattery12V, bool isBatteryLithium, uint32_t batteryEff, uint32_t batteryVoltage, uint32_t initialSoC)
{
    _batteryEff = batteryEff;
    _isBattery12V = isBattery12V;
    _isBatteryLithium = isBatteryLithium;

    // use stored soc, unless it's out of range, in which case calculate new starting point
    _previousSoC = (initialSoC >= 0 && initialSoC <= SOC_SCALED_MAX)
        ? initialSoC
        : calculateInitialSoC(batteryVoltage);

    _x[0] = _previousSoC;
    diagonalMatrix(_pval, _pPost);   // identity(n) * pval
    diagonalMatrix(_qval, _q);       // identity(n) * qval
    diagonalMatrix(1.0, _a);         // identity
}

uint32_t SoCKalman::read()
{
    // do not excede 0-100% bounds
    return clamp(_previousSoC, 0, SOC_SCALED_HUNDRED_PERCENT);
}

uint32_t SoCKalman::efficiency()
{
    return _batteryEff;
}

void SoCKalman::f(bool isBatteryInFloat, int32_t batteryMilliWatts, uint32_t samplePeriodMilliSec, uint32_t batteryCapacity)
{
    uint32_t milliSecToHours = 3600000;
    int32_t powerChange = ((batteryMilliWatts / 1000) * _batteryEff * (samplePeriodMilliSec / milliSecToHours));   // scaling should be fine here
    uint32_t newSoC = (_x[0] * batteryCapacity + powerChange) / batteryCapacity;                                   // scaling should be fine here

    _x[0] = newSoC;

    if (isBatteryInFloat) {
        _millisecondsInFloat += samplePeriodMilliSec;
        if (_millisecondsInFloat > _floatResetDuration) {
            _batteryEff = (uint64_t)_batteryEff * (uint64_t)SOC_SCALED_HUNDRED_PERCENT / _previousSoC;
            _batteryEff = clamp(_batteryEff, 0, SOC_SCALED_HUNDRED_PERCENT);
            _x[0] = SOC_SCALED_HUNDRED_PERCENT;
        }
    } else {
        _millisecondsInFloat = 0;
    }
}

void SoCKalman::h(int32_t batteryMilliAmps)
{
    // _h is the voltage that most closely matches current soc (a number)
    // _H is an array of form [ocv gradient, measured current, 1] (the last parameter is the offset)
    // x_[0] = SOC, _x[1] = R

    uint32_t dummyLeadAcidVoltage[101] = { 11640, 11653, 11666, 11679, 11692, 11706, 11719, 11732, 11745, 11758, 11772, 11785, 11798, 11811, 11824, 11838, 11851, 11864, 11877, 11890, 11904, 11917, 11930, 11943, 11956, 11970, 11983, 11996, 12009, 12022, 12036, 12049, 12062, 12075, 12088, 12102, 12115, 12128, 12141, 12154, 12168, 12181, 12194, 12207, 12220, 12234, 12247, 12260, 12273, 12286, 12300, 12313, 12326, 12339, 12352, 12366, 12379, 12392, 12405, 12418, 12432, 12445, 12458, 12471, 12484, 12498, 12511, 12524, 12537, 12550, 12564, 12577, 12590, 12603, 12616, 12630, 12643, 12656, 12669, 12682, 12696, 12709, 12722, 12735, 12748, 12762, 12775, 12788, 12801, 12814, 12828, 12841, 12854, 12867, 12880, 12894, 12907, 12920, 12933, 12946, 12960 };
    uint32_t dummyLithiumVoltage[101] = { 5000, 6266, 7434, 8085, 8531, 8867, 9134, 9355, 9543, 9705, 9847, 9974, 10088, 10191, 10285, 10372, 10451, 10525, 10595, 10659, 10720, 10777, 10831, 10882, 10931, 10977, 11021, 11063, 11104, 11142, 11180, 11216, 11251, 11284, 11317, 11349, 11379, 11409, 11438, 11467, 11495, 11522, 11548, 11574, 11600, 11625, 11650, 11675, 11699, 11723, 11746, 11769, 11793, 11815, 11838, 11861, 11883, 11906, 11928, 11950, 11972, 11994, 12017, 12039, 12061, 12083, 12105, 12127, 12150, 12172, 12195, 12217, 12240, 12263, 12286, 12309, 12333, 12356, 12380, 12404, 12428, 12452, 12477, 12501, 12526, 12552, 12577, 12603, 12629, 12655, 12682, 12708, 12735, 12763, 12790, 12818, 12846, 12875, 12903, 12931, 12960 };
    uint32_t dummyOcvSoc[101] = { 0, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 13000, 14000, 15000, 16000, 17000, 18000, 19000, 20000, 21000, 22000, 23000, 24000, 25000, 26000, 27000, 28000, 29000, 30000, 31000, 32000, 33000, 34000, 35000, 36000, 37000, 38000, 39000, 40000, 41000, 42000, 43000, 44000, 45000, 46000, 47000, 48000, 49000, 50000, 51000, 52000, 53000, 54000, 55000, 56000, 57000, 58000, 59000, 60000, 61000, 62000, 63000, 64000, 65000, 66000, 67000, 68000, 69000, 70000, 71000, 72000, 73000, 74000, 75000, 76000, 77000, 78000, 79000, 80000, 81000, 82000, 83000, 84000, 85000, 86000, 87000, 88000, 89000, 90000, 91000, 92000, 93000, 94000, 95000, 96000, 97000, 98000, 99000, 100000 };

    // update voltage closest to current state of charge as well as gradient
    int i;
    uint16_t multiplier;

    if (_isBattery12V) {
        multiplier = 1;
    } else {
        multiplier = 2;
    }

    for (i = 0; i < 101; i++) {
        if (dummyOcvSoc[i] > (uint32_t)_x[0]) {
            if (_isBatteryLithium) {
                _h = (dummyLithiumVoltage[i] + dummyLithiumVoltage[i - 1]) * multiplier / 2 + (batteryMilliAmps / 1000 * _x[1] / 100) + _x[2] / 100;   // units should be good here
                _H[0] = (dummyLithiumVoltage[i] - dummyLithiumVoltage[i - 1]) * multiplier * 100 / (dummyOcvSoc[i] - dummyOcvSoc[i - 1]);              // units are good here
            } else {
                _h = (dummyLeadAcidVoltage[i] + dummyLeadAcidVoltage[i - 1]) * multiplier / 2 + (batteryMilliAmps / 1000 * _x[1] / 100) + _x[2] / 100;
                _H[0] = (dummyLeadAcidVoltage[i] - dummyLeadAcidVoltage[i - 1]) * multiplier * 100 / (dummyOcvSoc[i] - dummyOcvSoc[i - 1]);
            }
            _H[1] = batteryMilliAmps / 1000;   // should be good in Amps
            _H[2] = 1;                         // offset
            return;
        }
    }
}

void SoCKalman::sample(bool isBatteryInFloat, int32_t batteryMilliAmps, uint32_t batteryVoltage, int32_t batteryMilliWatts, uint32_t samplePeriodMilliSec,
    uint32_t batteryCapacity)
{
    float temp0[_n * _n];
    float temp1[_n * _n];
    float temp2[_n * _m];
    float temp3[_n * _m];
    float temp4[_m * _m];
    float temp5;

    // $\hat{x}_k = f(\hat{x}_{k-1})$
    f(isBatteryInFloat, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);

    // $P_k = A_{k-1} P_{k-1} A^T_{k-1} + Q_{k-1}$ -- updates _pPre
    matMult(_a, _pPost, temp0, _n, _n, _n);
    transpose(_a, _at, _n, _n);
    matMult(temp0, _at, temp1, _n, _n, _n);
    matAdd(temp1, _q, _pPre, _n * _n);

    // update measurable (voltage) based on predicted state (SOC)
    h(batteryMilliAmps);

    // $G_k = P_k H^T_k (H_k P_k H^T_k + R)^{-1}$
    transpose(_H, _Ht, _m, _n);
    matMult(_pPre, _Ht, temp2, _n, _n, _m);
    matMult(_H, _pPre, temp3, _m, _n, _n);
    matMult(temp3, _Ht, temp4, _m, _n, _m);
    temp5 = 1 / (temp4[0] + _rval);
    matMultConst(temp2, temp5, _G, _n * _m);

    // $\hat{x}_k = \hat{x_k} + G_k(z_k - h(\hat{x}_k))$
    temp5 = batteryVoltage - _h;
    matMultConst(_G, temp5, temp3, _n * _m);
    updateState(temp3, _n * _m);
    _x[0] = clamp((uint32_t)_x[0], 0, SOC_SCALED_HUNDRED_PERCENT);

    // $P_k = (I - G_k H_k) P_k$
    matMult(_G, _H, temp0, _n, _m, _n);
    negate(temp0, _n * _n);
    diagonalMatrix(1.0, temp1);
    matAccum(temp0, temp1, _n * _n);
    matMult(temp0, _pPre, _pPost, _n, _n, _n);

    _previousSoC = _x[0];
}

uint32_t SoCKalman::calculateInitialSoC(uint32_t batteryVoltage)
{
    // will need to add 24 V compatability

    const uint8_t VOLTAGES_SIZE = 10;
    const uint16_t battSoCVoltages[VOLTAGES_SIZE] = { 12720, 12600, 12480, 12360, 12240, 12120, 12000, 11880, 11760, 11640 };

    uint8_t index;
    for (index = 0; index < VOLTAGES_SIZE; index++)
        if (batteryVoltage > battSoCVoltages[index])
            break;

    return (VOLTAGES_SIZE - index) * (SOC_SCALED_HUNDRED_PERCENT / VOLTAGES_SIZE);
}

void SoCKalman::diagonalMatrix(float value, float* result)
{
    int i, j;

    for (i = 0; i < _n; i++)
        for (j = 0; j < _n; j++) {
            if (i == j)
                result[i * _n + j] = value;
            else
                result[i * _n + j] = 0;
        }
}

void SoCKalman::matMult(float* a, float* b, float* result, uint8_t arows, uint8_t acols, uint8_t bcols)
{
    int i, j, k;

    for (i = 0; i < arows; i++)
        for (j = 0; j < bcols; j++) {
            result[i * bcols + j] = 0;
            for (k = 0; k < acols; k++)
                result[i * bcols + j] += a[i * acols + k] * b[k * bcols + j];
        }
}

void SoCKalman::matMultConst(float* a, float b, float* result, uint8_t length)
{
    int i;

    for (i = 0; i < length; i++) {
        result[i] = a[i] * b;
    }
}

void SoCKalman::matAdd(float* a, float* b, float* result, uint8_t length)
{
    int i;

    for (i = 0; i < length; i++) {
        result[i] = a[i] + b[i];
    }
}

void SoCKalman::matAccum(float* a, float* b, uint8_t length)
{
    // not tested directly but pretty simple
    int i;
    for (i = 0; i < length; i++) {
        a[i] = a[i] + b[i];
    }
}

void SoCKalman::transpose(float* a, float* result, uint8_t rows, uint8_t cols)
{
    int i, j;

    for (i = 0; i < rows; i++)
        for (j = 0; j < cols; j++) {
            result[j * rows + i] = a[i * cols + j];
        }
}

void SoCKalman::negate(float* a, uint8_t length)
{
    for (int i = 0; i < _n * _n; i++) {
        a[i] = -1 * a[i];
    }
}

void SoCKalman::updateState(float* a, uint8_t length)
{
    for (int i = 0; i < 3; i++) {
        _x[i] = _x[i] + a[i] * 100;
    }
}

uint8_t SoCKalman::inverse(float* a, float* result)
{
    int i, j;
    float determinant = 0;

    // find determinant first
    for (i = 0; i < 3; i++) {
        determinant = determinant + (a[i] * (a[3 + (i + 1) % 3] * a[6 + (i + 2) % 3] - a[3 + (i + 2) % 3] * a[6 + (i + 1) % 3]));
    }

    if (determinant == 0)
        return 1;

    // use cofactors to calculate resulting inverse
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            result[i * 3 + j] = ((a[((j + 1) % 3) * 3 + (i + 1) % 3] * a[((j + 2) % 3) * 3 + (i + 2) % 3]) - (a[((j + 1) % 3) * 3 + (i + 2) % 3] * a[((j + 2) % 3) * 3 + (i + 1) % 3])) / determinant;
        }
    }

    return 0;
}

uint32_t SoCKalman::clamp(uint32_t value, uint32_t min, uint32_t max)
{
    if (value > max) {
        return max;
    } else if (value < min) {
        return min;
    }
    return value;
}
