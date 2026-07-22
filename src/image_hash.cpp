#include "kyrase/image_hash.h"

void hash_byte(uint64_t& hash, const uint8_t value) {
	hash ^= value;
	hash *= FNV_PRIME;
}

void hash_int(
	uint64_t& hash,
	uint64_t value
) {
	for (int i = 0; i < 8; i++) {
		hash_byte(
			hash,
			static_cast<uint8_t>(value & 0xFF)
		);

		value >>= 8;
	}
}

void hash_channel(
	uint64_t& hash,
	const vector<uint8_t>& channel
) {
	for (const uint8_t value : channel) {
		hash_byte(hash, value);
	}
}

uint64_t hash_image_fnv1a(const Image& image) {
	uint64_t hash = FNV_OFFSET;

	hash_int(hash, static_cast<uint64_t>(image.get_width()));
	hash_int(hash, static_cast<uint64_t>(image.get_height()));
	hash_int(hash, static_cast<uint64_t>(image.get_channels()));
	hash_int(hash, static_cast<uint64_t>(image.get_max_color_val()));

	hash_channel(hash, image.get_r());
	hash_channel(hash, image.get_g());
	hash_channel(hash, image.get_b());

	return hash;
}