#include "crypto.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

using Byte = uint8_t;

static void get_random_bytes(uint8_t *buffer, size_t len) {
#ifdef _WIN32
    HCRYPTPROV hProv;
    CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
    CryptGenRandom(hProv, (DWORD)len, buffer);
    CryptReleaseContext(hProv, 0);
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t result = read(fd, buffer, len);
        (void)result;
        close(fd);
    }
#endif
}

static std::string generate_salt() {
    static const char ascii_letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string salt(16, '\0');
    uint8_t random_bytes[16];
    get_random_bytes(random_bytes, 16);
    salt[0] = ascii_letters[random_bytes[0] % 52];
    for (int i = 1; i < 16; i++) {
        salt[i] = (random_bytes[i] % (126 - 37)) + 37;
    }
    return salt;
}

static std::vector<Byte> sort_by_nth_element(const std::vector<Byte>& data, size_t n) {
    if (data.empty()) return {};
    std::vector<Byte> arr_copy = data;
    std::vector<Byte> result;
    size_t index = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        long long idx = static_cast<long long>(index) + static_cast<long long>(n) - 1;
        long long mod = idx % static_cast<long long>(arr_copy.size());
        if (mod < 0) mod += static_cast<long long>(arr_copy.size());
        index = static_cast<size_t>(mod);
        result.push_back(arr_copy[index]);
        arr_copy.erase(arr_copy.begin() + index);
    }
    return result;
}

static std::vector<Byte> latin1_to_utf8_bytes(const std::string& s) {
    std::vector<Byte> out;
    out.reserve(s.size());
    for (unsigned char uc : s) {
        if (uc < 0x80) {
            out.push_back(static_cast<Byte>(uc));
        } else if (uc < 0xC0) {
            out.push_back(static_cast<Byte>(0xC2));
            out.push_back(static_cast<Byte>(uc));
        } else {
            out.push_back(static_cast<Byte>(0xC3));
            out.push_back(static_cast<Byte>(uc - 0x40));
        }
    }
    return out;
}

static std::tuple<std::string, std::string, std::string, std::string, std::string, std::string>
internal_hash_password(const std::string& password, const std::string* salt_ptr, const std::vector<std::string>* memo) {
    std::string salt = salt_ptr ? *salt_ptr : "";
    std::string combined = salt + "$" + password;
    std::vector<Byte> combined_bytes(combined.begin(), combined.end());

    std::string hash1, hash2, hash3, hash4;
    int accumulator = static_cast<int>(combined_bytes[0]);
    size_t memo_length = memo ? memo->size() : 0;
    
    if (memo && memo_length > 0) {
        hash4 = (*memo)[memo_length - 1];
        int index = static_cast<int>(combined_bytes[combined_bytes.size() - 2]) * 97;
        index = index % static_cast<int>(memo_length);
        const std::string* string_memo = &(*memo)[index];
        size_t string_memo_length = string_memo->size();
        index = static_cast<int>(combined_bytes[combined_bytes.size() - 7]) * 113;
        index = index % static_cast<int>(string_memo_length);
        accumulator = static_cast<unsigned char>((*string_memo)[index]);
    } else {
        int index = static_cast<int>(combined_bytes[0]) % 90;
        accumulator = static_cast<Byte>(index + 37);
    }

    if (accumulator > 96)
        accumulator -= 70;
    else
        accumulator -= 64;

    for (size_t i = 0; i < combined.size(); ++i) {
        int hash_value = accumulator * 113;
        hash_value += combined_bytes[i];
        accumulator += 2;
        
        if (accumulator > 48)
            accumulator -= 23;

        char c = static_cast<char>(((hash_value % 90)) & (((hash_value / 90) % 90) + 37));
        hash1 += c;
    }
    
    std::vector<Byte> hash1_bytes = latin1_to_utf8_bytes(hash1);
    int sort_index = accumulator % 5;
    std::vector<Byte> sorted_hash_bytes = sort_by_nth_element(hash1_bytes, static_cast<size_t>(sort_index));

    int hash_value = 0;
    for (Byte byte_val : sorted_hash_bytes) {
        hash_value *= 71;
        hash_value += byte_val;
        
        while (hash_value > 128) {
            hash2 += static_cast<char>((hash_value % 90) + 37);
            hash_value = hash_value / 90;
        }
    }
    hash2 += static_cast<char>((hash_value % 90) + 37);

    for (Byte byte_val : sorted_hash_bytes) {
        hash_value *= 997;
        hash_value += byte_val;
        
        while (hash_value > 128) {
            hash3 += static_cast<char>((hash_value % 90) + 37);
            hash_value = hash_value / 90;
        }
    }
    hash3 += static_cast<char>((hash_value % 90) + 37);

    std::vector<Byte> hash_4_bytes;
    if (!hash4.empty()) {
        hash_4_bytes.assign(hash4.begin(), hash4.end());
    } else {
        hash_4_bytes = sorted_hash_bytes;
        hash4.clear();
    }
    
    int multiplier = (memo_length > 73) ? 1997 : 23;
    for (Byte byte_val : hash_4_bytes) {
        hash_value *= multiplier;
        hash_value += byte_val;
        
        while (hash_value > 128) {
            hash4 += static_cast<char>((hash_value % 90) + 37);
            hash_value = hash_value / 90;
        }
    }
    
    if (memo_length > 47) {
        return { "$" + salt + "$/$" + hash2, hash1, hash2, hash3, hash4, hash3 };
    } else {
        return { "$" + salt + "$/$" + hash2 + "$" + hash4, hash1, hash2, hash3, hash3, hash4 };
    }
}

char* hash_password(const char *password, int cost, const char *salt) {
    std::vector<std::string> memo;
    std::vector<std::string> password_tuple;
    std::string current_password = password;

    int iterations = 1 << cost;
    for (int i = 0; i < iterations; ++i) {
        password_tuple.clear();
        std::string local_salt = salt ? salt : generate_salt();
        auto hash_result = internal_hash_password(current_password, &local_salt, &memo);
        password_tuple.push_back(std::get<0>(hash_result));
        password_tuple.push_back(std::get<1>(hash_result));
        password_tuple.push_back(std::get<2>(hash_result));
        password_tuple.push_back(std::get<3>(hash_result));
        password_tuple.push_back(std::get<4>(hash_result));
        password_tuple.push_back(std::get<5>(hash_result));

        if (memo.size() < static_cast<size_t>(16 * (i + 1))) memo.resize(16 * (i + 1));
        memo[16 * i + 0] = password_tuple[0];
        memo[16 * i + 1] = password_tuple[1];
        memo[16 * i + 2] = password_tuple[2];
        memo[16 * i + 3] = password_tuple[3];
        memo[16 * i + 4] = password_tuple[4];
        memo[16 * i + 15] = password_tuple[5];
        memo[16 * i + 6] = password_tuple[5] + password_tuple[2];
        memo[16 * i + 7] = password_tuple[5] + password_tuple[3];
        memo[16 * i + 8] = password_tuple[5] + password_tuple[4];
        memo[16 * i + 9] = password_tuple[5] + password_tuple[1];
        memo[16 * i + 10] = password_tuple[5] + password_tuple[0];
        memo[16 * i + 11] = password_tuple[5] + password_tuple[2];
        memo[16 * i + 12] = password_tuple[2] + password_tuple[3];
        memo[16 * i + 13] = password_tuple[2] + password_tuple[4];
        memo[16 * i + 14] = password_tuple[0] + password_tuple[1];
        memo[16 * i + 5] = password_tuple[0] + password_tuple[3];

        current_password = password_tuple[0];
    }
    char* result = (char*)malloc(current_password.size() + 1);
    strcpy(result, current_password.c_str());
    return result;
}

int encrypt_file(const char *input_file, const char *output_file, const unsigned char *key, size_t key_len) {
    FILE *in = fopen(input_file, "rb");
    if (!in) return -1;
    
    FILE *out = fopen(output_file, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }
    
    int c;
    size_t key_idx = 0;
    while ((c = fgetc(in)) != EOF) {
        unsigned char encrypted = c ^ key[key_idx % key_len];
        fputc(encrypted, out);
        key_idx++;
    }
    
    fclose(in);
    fclose(out);
    return 0;
}

int decrypt_file(const char *input_file, const char *output_file, const unsigned char *key, size_t key_len) {
    FILE *in = fopen(input_file, "rb");
    if (!in) return -1;
    
    FILE *out = fopen(output_file, "wb");
    if (!out) {
        fclose(in);
        return -1;
    }
    
    int c;
    size_t key_idx = 0;
    while ((c = fgetc(in)) != EOF) {
        unsigned char decrypted = c ^ key[key_idx % key_len];
        fputc(decrypted, out);
        key_idx++;
    }
    
    fclose(in);
    fclose(out);
    return 0;
}
