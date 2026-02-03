# CRITICAL BUG FIX: Octane Treadmill ZR8 Speed Parsing

## Summary
Fixed a critical buffer mismatch bug in `octanetreadmill.cpp` that caused completely inaccurate speed values (0.156243 km/h, 14.0078 km/h, etc.).

## Bug Description

### Location
`src/devices/octanetreadmill/octanetreadmill.cpp`, lines 426-435 in `characteristicChanged()`

### The Problem
When extracting speed from non-standard packet formats (packets without `a5` header), the code was:
1. **Searching** for the pace marker byte (0x02 0x23, 0x01 0x23, or 0x00 0x23) in the **`newValue`** buffer
2. **Using** the resulting index to read bytes from the **`value`** buffer

```cpp
// ❌ BUGGY CODE
int16_t idx = newValue.indexOf(actualPaceSign) + 2;  // Find in newValue
if (idx <= 1)
    idx = newValue.indexOf(actualPace2Sign) + 2;
if (idx <= 1)
    idx = newValue.indexOf(actualPace3Sign) + 2;

// ... later ...
double candidateSpeed = GetSpeedFromPacket(value, idx);  // Apply to value!
```

### Why This Breaks
During **fragmented packet reassembly**, `newValue` and `value` have **different lengths**:
- `newValue`: Current BLE fragment (e.g., 20 bytes)
- `value`: Reassembled complete packet (e.g., 29 bytes from 2 fragments)

An index found at position 15 in a 20-byte `newValue` is **invalid** for a 29-byte `value` buffer!

### Observed Symptoms
From debug log `debug-Sun_Jan_18_21_04_26_2026.log`:
```
Current speed: 0.156243    ❌ Way too low (should be 4-25 km/h for running)
Current speed: 2.00669
Current speed: 0.0803571   ❌ Clearly impossible
Current speed: 0.0890032
Current speed: 14.0078     ❌ Narrow repeated values indicate off-by-one error
Current speed: 14.0625
```

These values indicate:
1. Index pointing to wrong bytes in the speed data
2. Reading garbage data or partially shifted data
3. Affecting workout accuracy and user experience

## The Fix

Changed all three marker searches from `newValue` to `value`:

```cpp
// ✓ FIXED CODE
int16_t idx = value.indexOf(actualPaceSign) + 2;      // Find in value
if (idx <= 1)
    idx = value.indexOf(actualPace2Sign) + 2;         // Find in value
if (idx <= 1)
    idx = value.indexOf(actualPace3Sign) + 2;         // Find in value

// ... later ...
double candidateSpeed = GetSpeedFromPacket(value, idx);  // Apply to same value
```

### Consistency with Speed Extraction
The main speed extraction path (lines 510-514) already uses `value` consistently:
```cpp
int16_t i = value.indexOf(actualPaceSign) + 2;    // ✓ Correct
// ...
double speed = GetSpeedFromPacket(value, i);       // ✓ Correct
```

The non-standard packet path (lines 426-432) **should have done the same**.

## Impact

### Critical: Lives and Safety
- Users rely on accurate speed data for:
  - **Workout monitoring** and form validation
  - **Heart rate zone calculations** (speed affects effort assessment)
  - **Distance/calorie calculations**
  - **Real-time coaching cues** based on pace

Inaccurate speeds (0.156 km/h vs actual 10 km/h) could cause:
- Missed heart rate warnings
- Incorrect form feedback
- Invalid workout data for training analysis
- Loss of trust in the app

### Testing Failure
The existing test suite **did not catch this bug** because:
- Tests used pre-calculated `testData` with correct values
- Tests never actually parsed fragmented packets from real BLE data
- No regression test for the fragmented packet code path

This is why tests passed despite broken code.

## Verification

### Before Fix
Log shows erratic speed values from fragmented packet handling.

### After Fix
Speed extraction should:
1. Cluster around reasonable human running speeds (4-25 km/h)
2. Show smooth variations matching actual treadmill data
3. Never produce values < 0.5 km/h or > 40 km/h for human running

## Related Tests Added

Added new test cases to `TestOctaneTreadmillZR8.h`:
- `TestSpeedExtractionWithFragmentedPackets()` - Validates fragmented packet handling
- `TestIndexMarkerConsistency()` - Ensures marker search uses correct buffer
- `TestLogRegressionSpeedValues()` - Catches if garbage speeds reappear

## Files Changed
- `src/devices/octanetreadmill/octanetreadmill.cpp` - Lines 426-432
- `tst/Devices/TestOctaneTreadmillZR8.h` - Added regression tests

## Commit Message
```
Fix critical speed parsing bug in Octane ZR8 fragmented packets

- Use value buffer consistently for marker byte search (was using newValue)
- During packet reassembly, newValue and value have different lengths
- Index found in newValue is invalid for value buffer, causing 0.156 km/h, 14.0 km/h garbage
- Added regression tests for fragmented packet handling
- CRITICAL: This was affecting real user data quality and safety
```

## Why Tests Didn't Catch This

The test architecture uses **static test data** rather than **actual packet parsing**:

```cpp
// TestOctaneTreadmillZR8.h - Line 58
static const std::vector<TestMetric> testData = {
    {108, 6.14},      // Pre-calculated: cadence, speed
    {150, 24.00},
    {106, 6.78},
    // ... 47 more pre-calculated samples
};
```

Tests validate the dataset is valid, but **never actually parse packets**:
```cpp
TEST_F(OctaneTreadmillZR8CadenceTest, TestMetricDatasetValidity) {
    // This validates testData itself is consistent
    // NOT that parsing produces correct values from packets
    for (const auto &sample : testData) {
        EXPECT_GE(sample.cadence, expectedCadenceMin);  // ✓ testData is valid
        EXPECT_LE(sample.speed_kmh, expectedSpeedMaxKmh); // ✓ testData is valid
        // But does parsing produce this? NO TEST!
    }
}
```

**Result**: Bug in actual parsing code goes undetected because tests only verify the test data is reasonable.

## Recommendation

Going forward, test suite should include:
1. **Real packet data** from actual device logs
2. **Fragmented packet scenarios** with multiple fragments
3. **Assertions on parsed values**, not just pre-calculated datasets
4. **Regression tests** for every bug fix

This is a safety-critical application. Tests must verify actual behavior, not just consistency of fixtures.
