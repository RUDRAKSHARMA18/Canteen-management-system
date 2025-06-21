/*
 * Copyright 2025 Runtime Assassins
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 
#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <cstdint>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <openssl/hmac.h>
#include <openssl/sha.h>

// SHA-256 implementation
class SHA256Hasher {
public:
    SHA256Hasher();
    void update(const uint8_t* data, size_t length);
    void update(const std::string& data);
    std::string final();

private:
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
    void transform(const uint8_t* data);
};

SHA256Hasher::SHA256Hasher() {
    state[0] = 0x6a09e667;
    state[1] = 0xbb67ae85;
    state[2] = 0x3c6ef372;
    state[3] = 0xa54ff53a;
    state[4] = 0x510e527f;
    state[5] = 0x9b05688c;
    state[6] = 0x1f83d9ab;
    state[7] = 0x5be0cd19;
    count = 0;
    std::fill(buffer, buffer + 64, 0);
}

void SHA256Hasher::update(const uint8_t* data, size_t length) {
    size_t i = 0;
    size_t j = count % 64;
    count += length;

    if (j + length < 64) {
        std::copy(data, data + length, buffer + j);
        return;
    }

    std::copy(data, data + (64 - j), buffer + j);
    transform(buffer);
    i += 64 - j;

    while (i + 63 < length) {
        transform(data + i);
        i += 64;
    }

    std::copy(data + i, data + length, buffer);
}

void SHA256Hasher::update(const std::string& data) {
    update(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
}

std::string SHA256Hasher::final() {
    uint8_t padding[64] = {0x80};
    uint64_t bits = count * 8;
    for (int i = 0; i < 8; ++i) {
        padding[56 + i] = (bits >> (56 - 8 * i)) & 0xff;
    }

    update(padding, count % 64 < 56 ? 64 - (count % 64) : 128 - (count % 64));

    char hash[65];
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 4; ++j) {
            snprintf(hash + (i * 8 + j * 2), 3, "%02x", (state[i] >> (24 - j * 8)) & 0xff);
        }
    }
    hash[64] = '\0';
    return std::string(hash);
}

void SHA256Hasher::transform(const uint8_t* data) {
    static const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    uint32_t w[64];
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 16; ++i) {
        w[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) | (data[i * 4 + 2] << 8) | data[i * 4 + 3];
    }
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = (w[i - 15] >> 7 | w[i - 15] << 25) ^ (w[i - 15] >> 18 | w[i - 15] << 14) ^ (w[i - 15] >> 3);
        uint32_t s1 = (w[i - 2] >> 17 | w[i - 2] << 15) ^ (w[i - 2] >> 19 | w[i - 2] << 13) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    for (int i = 0; i < 64; ++i) {
        uint32_t S1 = (e >> 6 | e << 26) ^ (e >> 11 | e << 21) ^ (e >> 25 | e << 7);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t temp1 = h + S1 + ch + K[i] + w[i];
        uint32_t S0 = (a >> 2 | a << 30) ^ (a >> 13 | a << 19) ^ (a >> 22 | a << 10);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

std::string sha256(const std::string& input) {
    SHA256Hasher sha;
    sha.update(input);
    return sha.final();
}

// HMAC-SHA1 using OpenSSL
std::vector<uint8_t> hmac_sha1(const std::vector<uint8_t>& key, const std::vector<uint8_t>& message) {
    std::vector<uint8_t> hash(SHA_DIGEST_LENGTH);
    unsigned int len;
    
    HMAC(EVP_sha1(), key.data(), key.size(), message.data(), message.size(), hash.data(), &len);
    
    std::cerr << "HMAC: Final hash: ";
    for (uint8_t b : hash) {
        std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
    }
    std::cerr << std::dec << std::endl;
    
    return hash;
}

// Base32 encoding
std::string generate_base32_secret() {
    static const char* base32_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::vector<uint8_t> raw_secret(10); // 10 bytes = 16 base32 chars
    for (auto& byte : raw_secret) {
        byte = dis(gen);
    }

    std::string base32;
    uint32_t buffer = 0;
    int bits = 0;
    for (uint8_t byte : raw_secret) {
        buffer = (buffer << 8) | byte;
        bits += 8;
        while (bits >= 5) {
            base32 += base32_chars[(buffer >> (bits - 5)) & 0x1F];
            bits -= 5;
            buffer &= (1 << bits) - 1;
        }
    }
    if (bits > 0) {
        base32 += base32_chars[(buffer << (5 - bits)) & 0x1F];
    }
    return base32;
}

std::vector<uint8_t> base32_decode(const std::string& base32) {
    static const char* base32_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    std::vector<uint8_t> result;
    uint64_t buffer = 0;
    int bits = 0;

    std::string cleaned_base32 = base32;
    cleaned_base32.erase(std::remove(cleaned_base32.begin(), cleaned_base32.end(), '='), cleaned_base32.end());

    for (char c : cleaned_base32) {
        char upper_c = std::toupper(c);
        const char* pos = std::strchr(base32_chars, upper_c);
        if (!pos) {
            std::cerr << "Invalid base32 character: " << c << std::endl;
            continue;
        }
        int val = pos - base32_chars;
        buffer = (buffer << 5) | val;
        bits += 5;
        if (bits >= 8) {
            result.push_back((buffer >> (bits - 8)) & 0xFF);
            bits -= 8;
            buffer &= (1ULL << bits) - 1;
        }
    }

    std::cerr << "Base32 decoded: ";
    for (uint8_t b : result) {
        std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
    }
    std::cerr << std::dec << std::endl;
    return result;
}

// TOTP generation
uint32_t generate_totp(const std::string& secret, int64_t time_step) {
    std::vector<uint8_t> key = base32_decode(secret);
    if (key.empty()) {
        std::cerr << "TOTP: Empty key after base32 decode" << std::endl;
        return 0;
    }

    // Encode time step as 8-byte big-endian
    std::vector<uint8_t> message(8);
    for (int i = 7; i >= 0; --i) {
        message[i] = (time_step >> (8 * (7 - i))) & 0xFF;
    }

    std::cerr << "TOTP: HMAC message (time step " << time_step << "): ";
    for (uint8_t b : message) {
        std::cerr << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
    }
    std::cerr << std::dec << std::endl;

    std::vector<uint8_t> hash = hmac_sha1(key, message);
    if (hash.size() < 20) {
        std::cerr << "TOTP: Invalid HMAC-SHA1 output size: " << hash.size() << std::endl;
        return 0;
    }

    // Dynamic offset and truncation per RFC 6238
    int offset = hash[hash.size() - 1] & 0x0F;
    uint32_t truncated = ((hash[offset] & 0x7F) << 24) |
                         ((hash[offset + 1] & 0xFF) << 16) |
                         ((hash[offset + 2] & 0xFF) << 8) |
                         (hash[offset + 3] & 0xFF);
    uint32_t code = truncated % 1000000;

    std::cerr << "TOTP: Generated code " << code << " (offset " << offset << ", truncated " << truncated << ")" << std::endl;
    return code;
}

bool verify_totp(const std::string& secret, const std::string& code) {
    if (secret.empty() || code.empty()) {
        std::cerr << "TOTP verification failed: empty secret or code" << std::endl;
        return false;
    }

    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    int64_t time_step = now / 30;
    std::cerr << "TOTP: Current time " << now << " seconds, time step " << time_step << std::endl;

    try {
        uint32_t input_code = std::stoi(code);
        std::cerr << "TOTP: Verifying input code " << input_code << std::endl;
        for (int i = -3; i <= 3; ++i) {
            uint32_t expected_code = generate_totp(secret, time_step + i);
            std::cerr << "TOTP: Checking time step " << time_step + i << ", expected code: " << expected_code << std::endl;
            if (input_code == expected_code) {
                std::cerr << "TOTP verification successful" << std::endl;
                return true;
            }
        }
        std::cerr << "TOTP verification failed: no matching code" << std::endl;
    } catch (...) {
        std::cerr << "TOTP verification failed: invalid code format" << std::endl;
        return false;
    }
    return false;
}

#endif