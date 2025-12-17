#include "encryption.h"
#include "crypto.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <zip.h> 

void rotate_left(uint8_t *data, size_t len, int k) {
    if (len == 0 || k == 0) return;
    k = k % (len * 8);
    if (k < 0) k += len * 8;
    
    size_t n = len * 8;
    std::vector<uint8_t> temp(data, data + len);
    uint64_t int_val = 0;
    
    for (size_t i = 0; i < len && i < 8; i++) {
        int_val = (int_val << 8) | temp[i];
    }
    
    if (len <= 8) {
        uint64_t rotated = ((int_val << k) | (int_val >> (n - k))) & ((1ULL << n) - 1);
        for (int i = len - 1; i >= 0; i--) {
            data[i] = rotated & 0xFF;
            rotated >>= 8;
        }
    } else {
        // For larger data, use byte-level rotation
        std::vector<uint8_t> result(len);
        size_t byte_shift = k / 8;
        int bit_shift = k % 8;
        
        for (size_t i = 0; i < len; i++) {
            size_t src_idx = (i + len - byte_shift) % len;
            result[i] = temp[src_idx];
        }
        
        if (bit_shift > 0) {
            uint8_t carry = 0;
            for (size_t i = 0; i < len; i++) {
                uint8_t new_carry = result[i] >> (8 - bit_shift);
                result[i] = (result[i] << bit_shift) | carry;
                carry = new_carry;
            }
            result[0] |= carry;
        }
        
        memcpy(data, result.data(), len);
    }
}

void rotate_right(uint8_t *data, size_t len, int k) {
    if (len == 0 || k == 0) return;
    k = k % (len * 8);
    if (k < 0) k += len * 8;
    
    size_t n = len * 8;
    std::vector<uint8_t> temp(data, data + len);
    uint64_t int_val = 0;
    
    for (size_t i = 0; i < len && i < 8; i++) {
        int_val = (int_val << 8) | temp[i];
    }
    
    if (len <= 8) {
        uint64_t rotated = ((int_val >> k) | (int_val << (n - k))) & ((1ULL << n) - 1);
        for (int i = len - 1; i >= 0; i--) {
            data[i] = rotated & 0xFF;
            rotated >>= 8;
        }
    } else {
        std::vector<uint8_t> result(len);
        size_t byte_shift = k / 8;
        int bit_shift = k % 8;
        
        for (size_t i = 0; i < len; i++) {
            size_t src_idx = (i + byte_shift) % len;
            result[i] = temp[src_idx];
        }
        
        if (bit_shift > 0) {
            uint8_t carry = 0;
            for (int i = len - 1; i >= 0; i--) {
                uint8_t new_carry = result[i] << (8 - bit_shift);
                result[i] = (result[i] >> bit_shift) | carry;
                carry = new_carry;
            }
            result[len - 1] |= carry;
        }
        
        memcpy(data, result.data(), len);
    }
}

void xor_bytes(uint8_t *data, size_t data_len, const uint8_t *key, size_t key_len) {
    for (size_t i = 0; i < data_len; i++) {
        data[i] ^= key[i % key_len];
    }
}

void byte_manipulations(uint8_t *data, size_t data_len, const uint8_t *key, size_t key_len, int iterat) {
    if (key_len < 6) return;
    iterat = 10 + (iterat % 6);
    int n = key[(key[1] + key[2]) % (key_len - 5)] % 7;
    
    for (int i = 0; i < iterat; i++) {
        rotate_left(data, data_len, 1 + ((n + i) % 7));
        xor_bytes(data, data_len, key, key_len);
    }
}

void byte_manipulations_reverse(uint8_t *data, size_t data_len, const uint8_t *key, size_t key_len, int iterat) {
    if (key_len < 6) return;
    iterat = 10 + (iterat % 6);
    int n = key[(key[1] + key[2]) % (key_len - 5)] % 7;
    
    for (int i = iterat - 1; i >= 0; i--) {
        xor_bytes(data, data_len, key, key_len);
        rotate_right(data, data_len, 1 + ((n + i) % 7));
    }
}

// Helper to determine chunk size based on file size
static size_t get_chunk_size(size_t file_size) {
    if (file_size < 5 * 1024 * 1024) {  // < 5MB
        return file_size;  // No chunking
    } else if (file_size < 50 * 1024 * 1024) {  // < 50MB
        return 512 * 1024;  // 512KB
    } else if (file_size < 200 * 1024 * 1024) {  // < 200MB
        return 2 * 1024 * 1024;  // 2MB
    } else {
        return 8 * 1024 * 1024;  // 8MB for very large files
    }
}

static const size_t SUB_CHUNK_SIZE = 1024;  // 1KB sub-chunks

struct ChunkData {
    std::vector<uint8_t> data;
    int index;
};

int encrypt_file_advanced(const char *input_file, const char *output_file, const char *password, int cost) {
    std::ifstream infile(input_file, std::ios::binary | std::ios::ate);
    if (!infile) return -1;
    
    size_t file_size = infile.tellg();
    infile.seekg(0);
    
    char *salt_ptr = hash_password("", 8, nullptr);
    if (!salt_ptr) return -1;
    
    std::string salt_str(salt_ptr);
    size_t salt_start = salt_str.find('$') + 1;
    size_t salt_end = salt_str.find('$', salt_start);
    std::string salt = salt_str.substr(salt_start, salt_end - salt_start);
    free(salt_ptr);
    
    char *hashed_password = hash_password(password, cost, salt.c_str());
    if (!hashed_password) return -1;
    
    char *hash_of_hash = hash_password(hashed_password, cost, salt.c_str());
    if (!hash_of_hash) {
        free(hashed_password);
        return -1;
    }
    
    int err = 0;
    zip_t *za = zip_open(output_file, ZIP_CREATE | ZIP_TRUNCATE, &err);
    if (!za) {
        free(hashed_password);
        free(hash_of_hash);
        return -1;
    }
    
    std::string filename = input_file;
    size_t last_slash = filename.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        filename = filename.substr(last_slash + 1);
    }
    
    std::ostringstream metadata;
    metadata << "file : " << filename << "\n"
             << "salt : " << salt << "\n"
             << "cost : " << cost << "\n"
             << "hash_verify : " << hash_of_hash << "\n";
    
    std::string metadata_str = metadata.str();
    std::vector<uint8_t> metadata_enc(metadata_str.begin(), metadata_str.end());
    byte_manipulations(metadata_enc.data(), metadata_enc.size(), 
                      (uint8_t*)hashed_password, strlen(hashed_password), 0);
    
    char *metadata_copy = (char*)malloc(metadata_str.size());
    memcpy(metadata_copy, metadata_str.c_str(), metadata_str.size());
    zip_source_t *s = zip_source_buffer(za, metadata_copy, metadata_str.size(), 1);
    if (s && zip_file_add(za, "filedata.crypt", s, ZIP_FL_OVERWRITE) < 0) {
        zip_source_free(s);
    }
    
    uint8_t *metadata_enc_copy = (uint8_t*)malloc(metadata_enc.size());
    memcpy(metadata_enc_copy, metadata_enc.data(), metadata_enc.size());
    s = zip_source_buffer(za, metadata_enc_copy, metadata_enc.size(), 1);
    if (s && zip_file_add(za, "filedata_enc.crypt", s, ZIP_FL_OVERWRITE) < 0) {
        zip_source_free(s);
    }
    
    size_t chunk_size = get_chunk_size(file_size);
    size_t num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 2;
    
    std::vector<ChunkData> all_chunks;
    int chunk_index = 0;
    
    while (infile) {
        std::vector<uint8_t> chunk(chunk_size);
        infile.read((char*)chunk.data(), chunk_size);
        size_t bytes_read = infile.gcount();
        if (bytes_read == 0) break;
        chunk.resize(bytes_read);
        all_chunks.push_back({chunk, chunk_index++});
    }
    infile.close();
    
    std::mutex chunks_mutex;
    std::vector<std::pair<int, std::vector<uint8_t>>> encrypted_chunks(all_chunks.size());
    
    auto process_chunk = [&](size_t start, size_t end) {
        for (size_t idx = start; idx < end && idx < all_chunks.size(); idx++) {
            std::vector<uint8_t> encrypted_chunk;
            for (size_t i = 0; i < all_chunks[idx].data.size(); i += SUB_CHUNK_SIZE) {
                size_t sub_size = std::min(SUB_CHUNK_SIZE, all_chunks[idx].data.size() - i);
                std::vector<uint8_t> sub_chunk(all_chunks[idx].data.begin() + i, 
                                               all_chunks[idx].data.begin() + i + sub_size);
                byte_manipulations(sub_chunk.data(), sub_chunk.size(),
                                 (uint8_t*)hashed_password, strlen(hashed_password),
                                 all_chunks[idx].index);
                encrypted_chunk.insert(encrypted_chunk.end(), sub_chunk.begin(), sub_chunk.end());
            }
            encrypted_chunks[idx] = {all_chunks[idx].index, encrypted_chunk};
        }
    };
    
    std::vector<std::thread> threads;
    size_t chunks_per_thread = (all_chunks.size() + num_threads - 1) / num_threads;
    for (size_t i = 0; i < num_threads; i++) {
        size_t start = i * chunks_per_thread;
        size_t end = std::min(start + chunks_per_thread, all_chunks.size());
        if (start < all_chunks.size()) {
            threads.emplace_back(process_chunk, start, end);
        }
    }
    
    for (auto &t : threads) {
        if (t.joinable()) t.join();
    }
    
    for (const auto &ec : encrypted_chunks) {
        std::string chunk_name = "filedata_chunk_" + std::to_string(ec.first) + ".crypt";
        uint8_t *chunk_copy = (uint8_t*)malloc(ec.second.size());
        memcpy(chunk_copy, ec.second.data(), ec.second.size());
        zip_source_t *s = zip_source_buffer(za, chunk_copy, ec.second.size(), 1);
        if (s) zip_file_add(za, chunk_name.c_str(), s, ZIP_FL_OVERWRITE);
    }
    
    zip_close(za);
    free(hashed_password);
    free(hash_of_hash);
    return 0;
}

int decrypt_file_advanced(const char *input_file, const char *output_file, const char *password) {
    int err = 0;
    zip_t *za = zip_open(input_file, ZIP_RDONLY, &err);
    if (!za) return -1;
    
    zip_stat_t st;
    if (zip_stat(za, "filedata.crypt", 0, &st) != 0) {
        zip_close(za);
        return -1;
    }
    
    zip_file_t *zf = zip_fopen(za, "filedata.crypt", 0);
    if (!zf) {
        zip_close(za);
        return -1;
    }
    
    std::vector<char> metadata_buf(st.size);
    zip_fread(zf, metadata_buf.data(), st.size);
    zip_fclose(zf);
    
    std::string metadata_str(metadata_buf.begin(), metadata_buf.end());
    std::istringstream iss(metadata_str);
    std::string line, filename, salt, hash_verify;
    int cost = 10;
    
    while (std::getline(iss, line)) {
        if (line.find("file : ") == 0) filename = line.substr(7);
        else if (line.find("salt : ") == 0) salt = line.substr(7);
        else if (line.find("cost : ") == 0) cost = std::stoi(line.substr(7));
        else if (line.find("hash_verify : ") == 0) hash_verify = line.substr(14);
    }
    
    char *hashed_password = hash_password(password, cost, salt.c_str());
    if (!hashed_password) {
        zip_close(za);
        return -1;
    }
    
    char *hash_of_hash = hash_password(hashed_password, cost, salt.c_str());
    if (!hash_of_hash || hash_verify != hash_of_hash) {
        free(hashed_password);
        if (hash_of_hash) free(hash_of_hash);
        zip_close(za);
        return -2;
    }
    free(hash_of_hash);
    
    if (zip_stat(za, "filedata_enc.crypt", 0, &st) != 0) {
        free(hashed_password);
        zip_close(za);
        return -1;
    }
    
    zf = zip_fopen(za, "filedata_enc.crypt", 0);
    if (!zf) {
        free(hashed_password);
        zip_close(za);
        return -1;
    }
    
    std::vector<uint8_t> metadata_enc(st.size);
    zip_fread(zf, metadata_enc.data(), st.size);
    zip_fclose(zf);
    
    byte_manipulations_reverse(metadata_enc.data(), metadata_enc.size(),
                               (uint8_t*)hashed_password, strlen(hashed_password), 0);
    
    std::string decrypted_metadata(metadata_enc.begin(), metadata_enc.end());
    if (decrypted_metadata.find(metadata_str) != 0) {
        free(hashed_password);
        zip_close(za);
        return -2;
    }
    
    std::ofstream outfile(output_file, std::ios::binary);
    if (!outfile) {
        free(hashed_password);
        zip_close(za);
        return -1;
    }
    
    zip_int64_t num_entries = zip_get_num_entries(za, 0);
    std::vector<std::pair<int, std::string>> chunks;
    
    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char *name = zip_get_name(za, i, 0);
        if (!name) continue;
        std::string fname(name);
        if (fname.find("filedata_chunk_") == 0) {
            size_t pos = fname.find_last_of('_');
            size_t dot = fname.find('.');
            int idx = std::stoi(fname.substr(pos + 1, dot - pos - 1));
            chunks.push_back({idx, fname});
        }
    }
    
    std::sort(chunks.begin(), chunks.end());
    
    for (const auto &chunk_info : chunks) {
        if (zip_stat(za, chunk_info.second.c_str(), 0, &st) != 0) continue;
        
        zf = zip_fopen(za, chunk_info.second.c_str(), 0);
        if (!zf) continue;
        
        std::vector<uint8_t> encrypted_chunk(st.size);
        zip_fread(zf, encrypted_chunk.data(), st.size);
        zip_fclose(zf);
        
        for (size_t i = 0; i < encrypted_chunk.size(); i += SUB_CHUNK_SIZE) {
            size_t sub_size = std::min(SUB_CHUNK_SIZE, encrypted_chunk.size() - i);
            byte_manipulations_reverse(encrypted_chunk.data() + i, sub_size,
                                      (uint8_t*)hashed_password, strlen(hashed_password),
                                      chunk_info.first);
        }
        
        outfile.write((char*)encrypted_chunk.data(), encrypted_chunk.size());
    }
    
    outfile.close();
    zip_close(za);
    free(hashed_password);
    return 0;
}
