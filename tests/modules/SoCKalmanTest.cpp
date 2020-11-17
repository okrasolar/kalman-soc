#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "SoCKalman.h"

TEST_GROUP(SoCKalmanTest){};

TEST(SoCKalmanTest, ShouldInitWithInitialSoC)
{
    SoCKalman kalman;

    uint32_t batteryEff = 10;
    uint32_t batteryVoltage = 12000;
    uint32_t initialSoC = 10;

    kalman.init(true, false, batteryEff, batteryVoltage, initialSoC);
    uint16_t result = kalman.read();

    CHECK_EQUAL(initialSoC, result);
}

TEST(SoCKalmanTest, ShouldInitWithCalculatedSoC)
{
    SoCKalman kalman;

    uint32_t batteryEff = 10;
    uint32_t batteryVoltage = 12000;
    uint32_t initialSoC = 0xFFFFFFFF;
    uint16_t expectedResult = 30000;

    kalman.init(true, false, batteryEff, batteryVoltage, initialSoC);
    uint16_t result = kalman.read();

    CHECK_EQUAL(expectedResult, result);
}

TEST(SoCKalmanTest, ShouldIncreaseSoCNoFloatLeadAcid12V)
{
    SoCKalman kalman;

    uint32_t batteryEff = 100000;   // 100 %
    uint32_t batteryVoltage = 12500;
    uint32_t initialSoC = 50000;
    int32_t batteryMilliAmps = 1000;
    int32_t batteryMilliWatts = 12500;
    uint32_t samplePeriodMilliSec = 3600000;
    uint32_t batteryCapacity = 50 * 12;   // 50 Ah, 12 V
    uint32_t expectedResult = 56201;

    kalman.init(true, false, batteryEff, batteryVoltage, initialSoC);
    kalman.sample(false, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
    uint32_t result = kalman.read();

    CHECK_EQUAL(expectedResult, result);
}

TEST(SoCKalmanTest, ShouldDecreaseSoCNoFloatLeadAcid12V)
{
    SoCKalman kalman;

    uint32_t batteryEff = 100000;   // 100 %
    uint32_t batteryVoltage = 12000;
    uint32_t initialSoC = 50000;
    int32_t batteryMilliAmps = -1000;
    int32_t batteryMilliWatts = -12000;
    uint32_t samplePeriodMilliSec = 3600000;
    uint32_t batteryCapacity = 50 * 12;   // 50 Ah, 12 V
    uint32_t expectedResult = 41023;

    kalman.init(true, false, batteryEff, batteryVoltage, initialSoC);
    kalman.sample(false, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
    uint32_t result = kalman.read();

    CHECK_EQUAL(expectedResult, result);
}

TEST(SoCKalmanTest, ShouldIncreaseSoCTwiceNoFloatLeadAcid12V)
{
    SoCKalman kalman;

    uint32_t batteryEff = 100000;   // 100 %
    uint32_t batteryVoltage = 12500;
    uint32_t initialSoC = 50000;
    int32_t batteryMilliAmps = 1000;
    int32_t batteryMilliWatts = 12500;
    uint32_t samplePeriodMilliSec = 3600000;
    uint32_t batteryCapacity = 50 * 12;   // 50 Ah, 12 V
    uint32_t expectedResult = 58272;

    kalman.init(true, false, batteryEff, batteryVoltage, initialSoC);
    kalman.sample(false, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
    kalman.sample(false, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
    uint32_t result = kalman.read();

    CHECK_EQUAL(expectedResult, result);
}

// TEST(SoCKalmanTest, ShouldFloatTimerTriggerLeadAcid12V)
// {
//     SoCKalman kalman;

//     uint32_t batteryEff = 100000;  // 100 %
//     uint32_t batteryVoltage = 12700;
//     uint32_t initialSoC = 50000;
//     int32_t batteryMilliAmps = 1000;
//     uint32_t batteryMilliWatts = 12700;
//     uint32_t samplePeriodMilliSec = 3600000;
//     uint32_t batteryCapacity = 50 * 12;   // 50 Ah, 12 V
//     uint32_t expectedResult = 100000;  // 100 %

//     mock().expectOneCall("HAL_GetTick").andReturnValue(0);        // get start time
//     mock().expectOneCall("HAL_GetTick").andReturnValue(600001);   // check value
//     mock().expectOneCall("HAL_GetTick").andReturnValue(600001);   // stop
//     mock().expectOneCall("HAL_GetTick").andReturnValue(600001);   // reset

//     kalman.init(true, false, batteryEff, batteryVoltage, initialSoC);
//     kalman.sample(true, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
//     kalman.sample(true, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
//     uint32_t result = kalman.read();

//     CHECK_EQUAL(expectedResult, result);
// }

TEST(SoCKalmanTest, ShouldIncreaseSoCNoFloatLithium12V)
{
    SoCKalman kalman;

    uint32_t batteryEff = 100000;   // 100 %
    uint32_t batteryVoltage = 12000;
    uint32_t initialSoC = 50000;
    int32_t batteryMilliAmps = 1000;
    int32_t batteryMilliWatts = 12000;
    uint32_t samplePeriodMilliSec = 3600000;
    uint32_t batteryCapacity = 50 * 12;   // 50 Ah, 12 V
    uint32_t expectedResult = 57600;

    kalman.init(true, true, batteryEff, batteryVoltage, initialSoC);
    kalman.sample(false, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
    uint32_t result = kalman.read();

    CHECK_EQUAL(expectedResult, result);
}

TEST(SoCKalmanTest, ShouldDecreaseSoCNoFloatLithium12V)
{
    SoCKalman kalman;

    uint32_t batteryEff = 100000;   // 100 %
    uint32_t batteryVoltage = 11500;
    uint32_t initialSoC = 50000;
    int32_t batteryMilliAmps = -1000;
    int32_t batteryMilliWatts = -11500;
    uint32_t samplePeriodMilliSec = 3600000;
    uint32_t batteryCapacity = 50 * 12;   // 50 Ah, 12 V
    uint32_t expectedResult = 42136;

    kalman.init(true, true, batteryEff, batteryVoltage, initialSoC);
    kalman.sample(false, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
    uint32_t result = kalman.read();

    CHECK_EQUAL(expectedResult, result);
}

TEST(SoCKalmanTest, ShouldIncreaseSoCNoFloatLeadAcid24V)
{
    SoCKalman kalman;

    uint32_t batteryEff = 100000;   // 100 %
    uint32_t batteryVoltage = 12500 * 2;
    uint32_t initialSoC = 50000;
    int32_t batteryMilliAmps = 1000;
    int32_t batteryMilliWatts = 12500 * 2;
    uint32_t samplePeriodMilliSec = 3600000;
    uint32_t batteryCapacity = 50 * 24;   // 50 Ah, 12 V
    uint32_t expectedResult = 61655;

    kalman.init(false, false, batteryEff, batteryVoltage, initialSoC);
    kalman.sample(false, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
    uint32_t result = kalman.read();

    CHECK_EQUAL(expectedResult, result);
}

TEST(SoCKalmanTest, ShouldNotOverflowMaxCalcs)
{
    SoCKalman kalman;

    uint32_t batteryEff = 100000;   // 100 %
    uint32_t batteryVoltage = 12500;
    uint32_t initialSoC = 100000;
    int32_t batteryMilliAmps = 10000;
    int32_t batteryMilliWatts = 125000;
    uint32_t samplePeriodMilliSec = 3600000;   // 1 hour sample time
    uint32_t batteryCapacity = 50 * 12;
    uint32_t expectedResult = 100000;   // 100 %

    kalman.init(true, false, batteryEff, batteryVoltage, initialSoC);
    kalman.sample(false, batteryMilliAmps, batteryVoltage, batteryMilliWatts, samplePeriodMilliSec, batteryCapacity);
    uint32_t result = kalman.read();

    CHECK_EQUAL(expectedResult, result);
}
