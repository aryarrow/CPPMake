#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>
namespace SHA256{
	struct sha256_ctx {
		uint32_t state[8];
		uint32_t total[2];
		size_t buflen;
		uint32_t buffer[32];
	};
	class SHA256 {
	public:
	SHA256(){
		main_ctx.state[0] = 0x6a09e667UL;
		main_ctx.state[1] = 0xbb67ae85UL;
		main_ctx.state[2] = 0x3c6ef372UL;
		main_ctx.state[3] = 0xa54ff53aUL;
		main_ctx.state[4] = 0x510e527fUL;
		main_ctx.state[5] = 0x9b05688cUL;
		main_ctx.state[6] = 0x1f83d9abUL;
		main_ctx.state[7] = 0x5be0cd19UL;

		main_ctx.total[0] = 0;
		main_ctx.total[1] = 0;

		main_ctx.buflen = 0;
	}

	void reset() {
		main_ctx.state[0] = 0x6a09e667UL;
		main_ctx.state[1] = 0xbb67ae85UL;
		main_ctx.state[2] = 0x3c6ef372UL;
		main_ctx.state[3] = 0xa54ff53aUL;
		main_ctx.state[4] = 0x510e527fUL;
		main_ctx.state[5] = 0x9b05688cUL;
		main_ctx.state[6] = 0x1f83d9abUL;
		main_ctx.state[7] = 0x5be0cd19UL;

		main_ctx.total[0] = 0;
		main_ctx.total[1] = 0;
		main_ctx.buflen = 0;
	}

	std::vector<uint8_t> finalize() {
    uint8_t* buffer = reinterpret_cast<uint8_t*>(main_ctx.buffer);

    size_t bytes = main_ctx.buflen;

    buffer[bytes++] = 0x80;

    if (bytes > 56)
    {
        std::memset(buffer + bytes, 0, 64 - bytes);
        processBlock(buffer);

        // Clear the next block.
        std::memset(buffer, 0, 64);
    }
    else
    {
        std::memset(buffer + bytes, 0, 56 - bytes);
    }

    uint64_t bitLength =
        (static_cast<uint64_t>(main_ctx.total[1]) << 32) |
        static_cast<uint64_t>(main_ctx.total[0]);

    bitLength *= 8;

	//i hate big endian, little endian is superior :D
    for (int i = 0; i < 8; ++i)
    {
        buffer[56 + i] =
            static_cast<uint8_t>(
                bitLength >> (56 - i * 8)
            );
    }

    // Process final block.
    processBlock(buffer);

    std::vector<uint8_t> digest(32);

    for (int i = 0; i < 8; ++i)
    {
        digest[i * 4 + 0] = static_cast<uint8_t>(main_ctx.state[i] >> 24);
        digest[i * 4 + 1] = static_cast<uint8_t>(main_ctx.state[i] >> 16);
        digest[i * 4 + 2] = static_cast<uint8_t>(main_ctx.state[i] >> 8);
		digest[i * 4 + 3] = static_cast<uint8_t>(main_ctx.state[i]);
	}
		return digest;
	}

	std::string SHA256_file(std::filesystem::path filePath){
		reset();
		update_file(filePath);
		auto digest=finalize();
		return stringifyDigest(digest);
	}
	
	void update_file(std::filesystem::path filePath){
		namespace fs=std::filesystem;
		if (!fs::exists(filePath)){
			throw std::runtime_error("Sha256 failed: path not found"+filePath.string());
		}
		std::ifstream fileinput(filePath.string(),std::ios::binary);
		if (!fileinput) {
			throw std::runtime_error("Failed to open file for sha256 hashing:"+filePath.string());
		}
		std::vector<uint8_t> buffer(64 * 1024);
		while (fileinput) {
			fileinput.read(reinterpret_cast<char*>(buffer.data()),buffer.size());
			std::streamsize bytesRead=fileinput.gcount();
			if (bytesRead>0){
				update(buffer.data(), static_cast<size_t>(bytesRead));
			}
		}
	}

	std::string stringifyDigest(const std::vector<uint8_t>& digest){
		std::stringstream strstream;
		for (uint8_t byte:digest) {
			strstream<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<int>(byte);
		}
		return strstream.str();
	}

	void update_string(const std::string& data){
		update(reinterpret_cast<const uint8_t*>(data.data()), data.size());
	}

	void update(const uint8_t* data,size_t len
	){
		uint32_t old=main_ctx.total[0];

		main_ctx.total[0]+=static_cast<uint32_t>(len);
		main_ctx.total[1]+=static_cast<uint32_t>(static_cast<uint64_t>(len)>>32);

		if (main_ctx.total[0] < old) {
			main_ctx.total[1]++;
		}

		if (main_ctx.buflen != 0)
		{
			size_t needed = 64 - main_ctx.buflen;

			if (len < needed)
			{
				std::memcpy(
					reinterpret_cast<uint8_t*>(main_ctx.buffer)
					+ main_ctx.buflen,
				data,
				len
				);

				main_ctx.buflen += len;
				return;
			}

			std::memcpy(
				reinterpret_cast<uint8_t*>(main_ctx.buffer)
				+ main_ctx.buflen,
			   data,
			   needed
			);

			processBlock(
				reinterpret_cast<const uint8_t*>(main_ctx.buffer)
			);

			main_ctx.buflen = 0;

			data += needed;
			len -= needed;
		}

		// Process complete 64-byte blocks directly.
		while (len >= 64)
		{
			processBlock(data);

			data += 64;
			len -= 64;
		}

		// Save remaining bytes.
		if (len > 0)
		{
			std::memcpy(main_ctx.buffer, data, len);
			main_ctx.buflen = len;
		}

	}

	void processBlock(const uint8_t* buffer){
		uint32_t W[64];

		for (int i = 0; i < 16; ++i)
		{
			W[i] =
			(static_cast<uint32_t>(buffer[i * 4 + 0]) << 24) |
			(static_cast<uint32_t>(buffer[i * 4 + 1]) << 16) |
			(static_cast<uint32_t>(buffer[i * 4 + 2]) << 8)  |
			static_cast<uint32_t>(buffer[i * 4 + 3]);
		}

		for (int i=16;i<64;++i){
			W[i]=sigma1(W[i-2])+W[i-7]+sigma0(W[i-15])+W[i-16];
		}

		uint32_t a = main_ctx.state[0];
		uint32_t b = main_ctx.state[1];
		uint32_t c = main_ctx.state[2];
		uint32_t d = main_ctx.state[3];
		uint32_t e = main_ctx.state[4];
		uint32_t f = main_ctx.state[5];
		uint32_t g = main_ctx.state[6];
		uint32_t h = main_ctx.state[7];

		for (int i = 0; i < 64; ++i)
		{
			uint32_t t1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i];
			uint32_t t2 = Sigma0(a) + Maj(a, b, c);

			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		main_ctx.state[0] += a;
		main_ctx.state[1] += b;
		main_ctx.state[2] += c;
		main_ctx.state[3] += d;
		main_ctx.state[4] += e;
		main_ctx.state[5] += f;
		main_ctx.state[6] += g;
		main_ctx.state[7] += h;

	}

	private:
	sha256_ctx main_ctx;

	static uint32_t rotr(uint32_t x, uint32_t n)
	{
		return (x >> n) | (x << (32 - n));
	}

	static uint32_t Ch(uint32_t x, uint32_t y, uint32_t z)
	{
		return (x & y) ^ (~x & z);
	}

	static uint32_t Maj(uint32_t x, uint32_t y, uint32_t z)
	{
		return (x & y) ^ (x & z) ^ (y & z);
	}

	static uint32_t Sigma0(uint32_t x)
	{
		return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
	}

	static uint32_t Sigma1(uint32_t x)
	{
		return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
	}

	static uint32_t sigma0(uint32_t x)
	{
		return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
	}

	static uint32_t sigma1(uint32_t x)
	{
		return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
	}

	const uint32_t K[64] = {
		0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL,
		0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL,
		0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL,
		0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL,
		0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
		0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL,
		0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL,
		0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL,
		0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL,
		0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
		0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL,
		0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL,
		0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL,
		0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL,
		0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
		0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL,
	};
	};
};


