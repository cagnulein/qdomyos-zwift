#include "sramAXSCrypto.h"

#include <array>
#include <cstdint>
#include <cstring>

namespace {

using Block = std::array<uint8_t, 16>;

static const uint8_t sbox[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16};
static const uint8_t rcon[10] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};

uint8_t xtime(uint8_t a) { return static_cast<uint8_t>(((a << 1) ^ ((a & 0x80) ? 0x1b : 0)) & 0xff); }

class Aes128 {
  public:
    explicit Aes128(const QByteArray &key) {
        for (int i = 0; i < 16; ++i)
            roundKeys[static_cast<size_t>(i)] = static_cast<uint8_t>(key.at(i));
        for (int i = 4; i < 44; ++i) {
            uint8_t t[4];
            std::memcpy(t, &roundKeys[static_cast<size_t>((i - 1) * 4)], 4);
            if ((i % 4) == 0) {
                const uint8_t first = t[0];
                t[0] = sbox[t[1]] ^ rcon[(i / 4) - 1];
                t[1] = sbox[t[2]];
                t[2] = sbox[t[3]];
                t[3] = sbox[first];
            }
            for (int j = 0; j < 4; ++j)
                roundKeys[static_cast<size_t>(i * 4 + j)] =
                    roundKeys[static_cast<size_t>((i - 4) * 4 + j)] ^ t[j];
        }
    }

    Block encrypt(const Block &input) const {
        Block state = input;
        addRoundKey(state, 0);
        for (int round = 1; round < 10; ++round) {
            for (uint8_t &b : state) b = sbox[b];
            shiftRows(state);
            mixColumns(state);
            addRoundKey(state, round);
        }
        for (uint8_t &b : state) b = sbox[b];
        shiftRows(state);
        addRoundKey(state, 10);
        return state;
    }

  private:
    std::array<uint8_t, 176> roundKeys{};

    void addRoundKey(Block &state, int round) const {
        for (int i = 0; i < 16; ++i) state[static_cast<size_t>(i)] ^= roundKeys[static_cast<size_t>(round * 16 + i)];
    }

    static void shiftRows(Block &s) {
        uint8_t t = s[1]; s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = t;
        t = s[2]; s[2] = s[10]; s[10] = t; t = s[6]; s[6] = s[14]; s[14] = t;
        t = s[15]; s[15] = s[11]; s[11] = s[7]; s[7] = s[3]; s[3] = t;
    }

    static void mixColumns(Block &s) {
        for (int c = 0; c < 4; ++c) {
            const int i = c * 4;
            const uint8_t a0 = s[static_cast<size_t>(i)], a1 = s[static_cast<size_t>(i + 1)],
                          a2 = s[static_cast<size_t>(i + 2)], a3 = s[static_cast<size_t>(i + 3)];
            s[static_cast<size_t>(i)] = xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3;
            s[static_cast<size_t>(i + 1)] = a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3;
            s[static_cast<size_t>(i + 2)] = a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3);
            s[static_cast<size_t>(i + 3)] = (xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3);
        }
    }
};

Block blockFrom(const QByteArray &bytes, int offset = 0) {
    Block out{};
    for (int i = 0; i < 16 && offset + i < bytes.size(); ++i) out[static_cast<size_t>(i)] = static_cast<uint8_t>(bytes.at(offset + i));
    return out;
}

QByteArray blockTo(const Block &block) {
    QByteArray out(16, Qt::Uninitialized);
    for (int i = 0; i < 16; ++i) out[i] = static_cast<char>(block[static_cast<size_t>(i)]);
    return out;
}

void xorBlock(Block &a, const Block &b) { for (int i = 0; i < 16; ++i) a[static_cast<size_t>(i)] ^= b[static_cast<size_t>(i)]; }

Block shiftLeft(const Block &input) {
    Block out{};
    uint8_t carry = 0;
    for (int i = 15; i >= 0; --i) {
        out[static_cast<size_t>(i)] = static_cast<uint8_t>((input[static_cast<size_t>(i)] << 1) | carry);
        carry = static_cast<uint8_t>(input[static_cast<size_t>(i)] >> 7);
    }
    return out;
}

Block cmac(const Aes128 &aes, const QByteArray &message) {
    Block l = aes.encrypt(Block{});
    Block k1 = shiftLeft(l); if (l[0] & 0x80) k1[15] ^= 0x87;
    Block k2 = shiftLeft(k1); if (k1[0] & 0x80) k2[15] ^= 0x87;

    const int blocks = qMax(1, (message.size() + 15) / 16);
    const bool complete = message.size() > 0 && (message.size() % 16) == 0;
    Block x{};
    for (int i = 0; i < blocks - 1; ++i) {
        xorBlock(x, blockFrom(message, i * 16));
        x = aes.encrypt(x);
    }
    Block last = blockFrom(message, (blocks - 1) * 16);
    const int remainder = message.size() - (blocks - 1) * 16;
    if (complete) xorBlock(last, k1);
    else { last[static_cast<size_t>(remainder)] = 0x80; xorBlock(last, k2); }
    xorBlock(x, last);
    return aes.encrypt(x);
}

Block omac(const Aes128 &aes, uint8_t type, const QByteArray &data) {
    QByteArray prefixed(16, '\0');
    prefixed[15] = static_cast<char>(type);
    prefixed.append(data);
    return cmac(aes, prefixed);
}

QByteArray ctr(const Aes128 &aes, const Block &iv, const QByteArray &data) {
    QByteArray out(data.size(), Qt::Uninitialized);
    Block counter = iv;
    for (int offset = 0; offset < data.size(); offset += 16) {
        const Block stream = aes.encrypt(counter);
        const int count = qMin(16, data.size() - offset);
        for (int i = 0; i < count; ++i) out[offset + i] = static_cast<char>(static_cast<uint8_t>(data.at(offset + i)) ^ stream[static_cast<size_t>(i)]);
        for (int i = 15; i >= 0; --i) { counter[static_cast<size_t>(i)]++; if (counter[static_cast<size_t>(i)] != 0) break; }
    }
    return out;
}

bool equalTag(const Block &a, const QByteArray &b) {
    if (b.size() != 16) return false;
    uint8_t difference = 0;
    for (int i = 0; i < 16; ++i) difference |= a[static_cast<size_t>(i)] ^ static_cast<uint8_t>(b.at(i));
    return difference == 0;
}

struct U128 { uint32_t w[4]{}; };

U128 fromBigEndian(const QByteArray &bytes) {
    U128 out{};
    for (int i = 0; i < 16; ++i) out.w[i / 4] = (out.w[i / 4] << 8) | static_cast<uint8_t>(bytes.at(i));
    std::swap(out.w[0], out.w[3]); std::swap(out.w[1], out.w[2]);
    return out;
}

QByteArray toBigEndian(const U128 &value) {
    QByteArray out(16, Qt::Uninitialized);
    for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j)
        out[i * 4 + j] = static_cast<char>(value.w[3 - i] >> (24 - j * 8));
    return out;
}

int compare(const U128 &a, const U128 &b) {
    for (int i = 3; i >= 0; --i) if (a.w[i] != b.w[i]) return a.w[i] < b.w[i] ? -1 : 1;
    return 0;
}

void subtract(U128 &a, const U128 &b) {
    uint64_t borrow = 0;
    for (int i = 0; i < 4; ++i) {
        const uint64_t sub = static_cast<uint64_t>(b.w[i]) + borrow;
        const uint64_t old = a.w[i];
        a.w[i] = static_cast<uint32_t>(old - sub);
        borrow = old < sub ? 1 : 0;
    }
}

U128 reduceProduct(const uint32_t product[8]) {
    U128 result{};
    uint64_t carry = 0;
    for (int i = 0; i < 4; ++i) {
        const uint64_t value = static_cast<uint64_t>(product[i]) + static_cast<uint64_t>(product[i + 4]) * 713 + carry;
        result.w[i] = static_cast<uint32_t>(value);
        carry = value >> 32;
    }
    uint64_t high = carry;
    while (high != 0) {
        carry = 0;
        for (int i = 0; i < 4; ++i) {
            const uint64_t value = static_cast<uint64_t>(result.w[i]) + (i == 0 ? high * 713 : 0) + carry;
            result.w[i] = static_cast<uint32_t>(value);
            carry = value >> 32;
        }
        high = carry;
    }
    const U128 modulus{{0xfffffd37u, 0xffffffffu, 0xffffffffu, 0xffffffffu}};
    while (compare(result, modulus) >= 0) subtract(result, modulus);
    return result;
}

U128 multiplyMod(const U128 &a, const U128 &b) {
    uint32_t product[8]{};
    for (int i = 0; i < 4; ++i) {
        uint64_t carry = 0;
        for (int j = 0; j < 4; ++j) {
            const uint64_t value = static_cast<uint64_t>(a.w[i]) * b.w[j] + product[i + j] + carry;
            product[i + j] = static_cast<uint32_t>(value);
            carry = value >> 32;
        }
        int k = i + 4;
        while (carry && k < 8) {
            const uint64_t value = static_cast<uint64_t>(product[k]) + carry;
            product[k] = static_cast<uint32_t>(value);
            carry = value >> 32;
            ++k;
        }
    }
    return reduceProduct(product);
}

U128 powerMod(U128 base, const QByteArray &littleEndianExponent) {
    U128 result{{1, 0, 0, 0}};
    for (int byte = 0; byte < 16; ++byte) {
        const uint8_t bits = static_cast<uint8_t>(littleEndianExponent.at(byte));
        for (int bit = 0; bit < 8; ++bit) {
            if (bits & (1u << bit)) result = multiplyMod(result, base);
            base = multiplyMod(base, base);
        }
    }
    return result;
}

} // namespace

namespace sramaxscrypto {

QByteArray eaxEncrypt(const QByteArray &key, const QByteArray &nonce, const QByteArray &message) {
    if (key.size() != 16 || nonce.size() != 16) return QByteArray();
    const Aes128 aes(key);
    const Block nonceTag = omac(aes, 0, nonce);
    const Block headerTag = omac(aes, 1, QByteArray());
    const QByteArray ciphertext = ctr(aes, nonceTag, message);
    const Block ciphertextTag = omac(aes, 2, ciphertext);
    Block tag = nonceTag; xorBlock(tag, ciphertextTag); xorBlock(tag, headerTag);
    return ciphertext + blockTo(tag);
}

QByteArray eaxDecrypt(const QByteArray &key, const QByteArray &nonce, const QByteArray &ciphertextAndTag,
                     bool *authenticated) {
    if (authenticated) *authenticated = false;
    if (key.size() != 16 || nonce.size() != 16 || ciphertextAndTag.size() < 16) return QByteArray();
    const Aes128 aes(key);
    const QByteArray ciphertext = ciphertextAndTag.left(ciphertextAndTag.size() - 16);
    const QByteArray receivedTag = ciphertextAndTag.right(16);
    const Block nonceTag = omac(aes, 0, nonce);
    const Block headerTag = omac(aes, 1, QByteArray());
    const Block ciphertextTag = omac(aes, 2, ciphertext);
    Block tag = nonceTag; xorBlock(tag, ciphertextTag); xorBlock(tag, headerTag);
    if (!equalTag(tag, receivedTag)) return QByteArray();
    if (authenticated) *authenticated = true;
    return ctr(aes, nonceTag, ciphertext);
}

QByteArray computePublicKey(const QByteArray &privateKey) {
    if (privateKey.size() != 16) return QByteArray();
    U128 generator{{5, 0, 0, 0}};
    return toBigEndian(powerMod(generator, privateKey));
}

QByteArray computeSharedSecret(const QByteArray &privateKey, const QByteArray &devicePublicKey) {
    if (privateKey.size() != 16 || devicePublicKey.size() != 16) return QByteArray();
    return toBigEndian(powerMod(fromBigEndian(devicePublicKey), privateKey));
}

QByteArray decryptTransportedKey(const QByteArray &sharedSecret, const QByteArray &transportBlob, bool *authenticated) {
    if (transportBlob.size() != 48) { if (authenticated) *authenticated = false; return QByteArray(); }
    return eaxDecrypt(sharedSecret, transportBlob.left(16), transportBlob.mid(16), authenticated);
}

} // namespace sramaxscrypto
