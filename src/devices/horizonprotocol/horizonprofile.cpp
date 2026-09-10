#include "horizonprofile.h"

#include <cassert>

// CRC-CCITT lookup table, moved here with the routine that uses it.
const int CRC_TABLE[256] = {
    0,     4129,  8258,  12387, 16516, 20645, 24774, 28903, 33032, 37161, 41290, 45419, 49548, 53677, 57806, 61935,
    4657,  528,   12915, 8786,  21173, 17044, 29431, 25302, 37689, 33560, 45947, 41818, 54205, 50076, 62463, 58334,
    9314,  13379, 1056,  5121,  25830, 29895, 17572, 21637, 42346, 46411, 34088, 38153, 58862, 62927, 50604, 54669,
    13907, 9842,  5649,  1584,  30423, 26358, 22165, 18100, 46939, 42874, 38681, 34616, 63455, 59390, 55197, 51132,
    18628, 22757, 26758, 30887, 2112,  6241,  10242, 14371, 51660, 55789, 59790, 63919, 35144, 39273, 43274, 47403,
    23285, 19156, 31415, 27286, 6769,  2640,  14899, 10770, 56317, 52188, 64447, 60318, 39801, 35672, 47931, 43802,
    27814, 31879, 19684, 23749, 11298, 15363, 3168,  7233,  60846, 64911, 52716, 56781, 44330, 48395, 36200, 40265,
    32407, 28342, 24277, 20212, 15891, 11826, 7761,  3696,  65439, 61374, 57309, 53244, 48923, 44858, 40793, 36728,
    37256, 33193, 45514, 41451, 53516, 49453, 61774, 57711, 4224,  161,   12482, 8419,  20484, 16421, 28742, 24679,
    33721, 37784, 41979, 46042, 49981, 54044, 58239, 62302, 689,   4752,  8947,  13010, 16949, 21012, 25207, 29270,
    46570, 42443, 38312, 34185, 62830, 58703, 54572, 50445, 13538, 9411,  5280,  1153,  29798, 25671, 21540, 17413,
    42971, 47098, 34713, 38840, 59231, 63358, 50973, 55100, 9939,  14066, 1681,  5808,  26199, 30326, 17941, 22068,
    55628, 51565, 63758, 59695, 39368, 35305, 47498, 43435, 22596, 18533, 30726, 26663, 6336,  2273,  14466, 10403,
    52093, 56156, 60223, 64286, 35833, 39896, 43963, 48026, 19061, 23124, 27191, 31254, 2801,  6864,  10931, 14994,
    64814, 60687, 56684, 52557, 48554, 44427, 40424, 36297, 31782, 27655, 23652, 19525, 15522, 11395, 7392,  3265,
    61215, 65342, 53085, 57212, 44955, 49082, 36825, 40952, 28183, 32310, 20053, 24180, 11923, 16050, 3793,  7920,
};

int horizonprofile::GenerateCRC_CCITT(uint8_t *PUPtr8, int PU16_Count, int crcStart) {
    if (PU16_Count == 0) {
        return 0;
    }
    int crc = crcStart;
    for (int i = 0; i < PU16_Count; i++) {
        int c = CRC_TABLE[((crc & 65280) >> 8) ^ ((PUPtr8[i] & 255) & 255)];
        crc = ((crc << 8) & 65280) ^ c;
    }
    return crc;
}

void horizonprofile::updateProfileCRC() {
    int confirm = GenerateCRC_CCITT(&initData7[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9, sizeof(initData9), confirm);
    confirm = GenerateCRC_CCITT(initData10, sizeof(initData10), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7[8] = (confirm & 0xff);
    initData7[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_1[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_1, sizeof(initData9_1), confirm);
    confirm = GenerateCRC_CCITT(initData10_1, sizeof(initData10_1), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_1[8] = (confirm & 0xff);
    initData7_1[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_2[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_2, sizeof(initData9_2), confirm);
    confirm = GenerateCRC_CCITT(initData10_2, sizeof(initData10_2), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_2[8] = (confirm & 0xff);
    initData7_2[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_3[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_3, sizeof(initData9_3), confirm);
    confirm = GenerateCRC_CCITT(initData10_3, sizeof(initData10_3), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_3[8] = (confirm & 0xff);
    initData7_3[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_4[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_4, sizeof(initData9_4), confirm);
    confirm = GenerateCRC_CCITT(initData10_4, sizeof(initData10_4), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_4[8] = (confirm & 0xff);
    initData7_4[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_5[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_5, sizeof(initData9_5), confirm);
    confirm = GenerateCRC_CCITT(initData10_5, sizeof(initData10_5), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_5[8] = (confirm & 0xff);
    initData7_5[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_6[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_6, sizeof(initData9_6), confirm);
    confirm = GenerateCRC_CCITT(initData10_6, sizeof(initData10_6), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_6[8] = (confirm & 0xff);
    initData7_6[9] = (confirm >> 8);
}

void horizonprofile::testProfileCRC() {
    int confirm = GenerateCRC_CCITT(&initData7[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9, sizeof(initData9), confirm);
    confirm = GenerateCRC_CCITT(initData10, sizeof(initData10), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7[8] == (confirm & 0xff));
    assert(initData7[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_1[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_1, sizeof(initData9_1), confirm);
    confirm = GenerateCRC_CCITT(initData10_1, sizeof(initData10_1), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_1[8] == (confirm & 0xff));
    assert(initData7_1[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_2[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_2, sizeof(initData9_2), confirm);
    confirm = GenerateCRC_CCITT(initData10_2, sizeof(initData10_2), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_2[8] == (confirm & 0xff));
    assert(initData7_2[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_3[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_3, sizeof(initData9_3), confirm);
    confirm = GenerateCRC_CCITT(initData10_3, sizeof(initData10_3), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_3[8] == (confirm & 0xff));
    assert(initData7_3[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_4[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_4, sizeof(initData9_4), confirm);
    confirm = GenerateCRC_CCITT(initData10_4, sizeof(initData10_4), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_4[8] == (confirm & 0xff));
    assert(initData7_4[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_5[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_5, sizeof(initData9_5), confirm);
    confirm = GenerateCRC_CCITT(initData10_5, sizeof(initData10_5), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_5[8] == (confirm & 0xff));
    assert(initData7_5[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_6[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_6, sizeof(initData9_6), confirm);
    confirm = GenerateCRC_CCITT(initData10_6, sizeof(initData10_6), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_6[8] == (confirm & 0xff));
    assert(initData7_6[9] == (confirm >> 8));
}


// Poll and tail frames. horizontreadmill declares these as locals inside btinit(); they are
// duplicated here so the table is self-contained.
static uint8_t hsPoll1[] = {0x55, 0xaa, 0x09, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
static uint8_t hsPoll2[] = {0x55, 0xaa, 0x0a, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
static uint8_t hsPoll3[] = {0x55, 0xaa, 0x0b, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
static uint8_t hsPoll4[] = {0x55, 0xaa, 0x0c, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
static uint8_t hsPoll5[] = {0x55, 0xaa, 0x0d, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
static uint8_t hsStart[] = {0x55, 0xaa, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00};

std::vector<horizonprofile::frame> horizonprofile::handshakeFrames() {
    std::vector<frame> f;
    auto add = [&](uint8_t *d, uint8_t len, int times = 1) {
        for (int i = 0; i < times; i++)
            f.push_back(frame(d, len));
    };

    add(hsStart, sizeof(hsStart));

    uint8_t *g7[]  = {initData7,  initData7_1,  initData7_2,  initData7_3,  initData7_4,  initData7_5,  initData7_6};
    uint8_t *g9[]  = {initData9,  initData9_1,  initData9_2,  initData9_3,  initData9_4,  initData9_5,  initData9_6};
    uint8_t *g10[] = {initData10, initData10_1, initData10_2, initData10_3, initData10_4, initData10_5, initData10_6};

    for (int i = 0; i < 7; i++) {
        add(g7[i], 20);
        add(initData8, 20, 2);
        add(g9[i], 20);
        add(g10[i], 20);
        add(initData11, 20, 17);
        add(initData12, 20);
        add(initData8, 20, 13);
        add(initData13, 20);
        add(initData14, sizeof(initData14));
    }

    add(hsPoll1, sizeof(hsPoll1));
    add(hsPoll2, sizeof(hsPoll2));
    add(hsPoll3, sizeof(hsPoll3));
    add(hsPoll4, sizeof(hsPoll4));
    add(hsPoll5, sizeof(hsPoll5));
    return f;
}
