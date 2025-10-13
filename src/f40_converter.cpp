
// *********
// f40_converter.cpp
// *********

#include "f40_converter.hpp"
#include <cstring> // Per memcpy
#include <format>

// Le funzioni originali sono state rese statiche per limitarne la visibilità a questo file.
namespace 
{
	// Converte un double da formato IEEE 754 a Microsoft Binary Format (8 byte intermedi)
	int convert_dieeetomsbin(const double src8, double *dest8)
	{
		unsigned char ieee[8];
		unsigned char *msbin = (unsigned char *)dest8;
		unsigned char sign;
		unsigned char any_on = 0;
		short msbin_exp;
		int i;

		memcpy(ieee, &src8, 8);
		for (i = 0; i < 8; i++) msbin[i] = 0;

		for (i = 0; i < 8; i++) any_on |= ieee[i];
		if (any_on == 0) return 0;

		sign = ieee[7] & 0x80;
		msbin[6] |= sign;

		msbin_exp = (ieee[7] & 0x7f) << 4;
		msbin_exp += (ieee[6] >> 4);

		if ((msbin_exp - 0x3ff) > 0x80) return 1;

		msbin[7] = static_cast<unsigned char>(msbin_exp - 0x3ff + 0x80 + 1);

		ieee[6] &= 0x0f;
		for (i = 6; i >= 1; i--) {
			msbin[i] |= (ieee[i] << 3);
			msbin[i] |= (ieee[i - 1] >> 5);
		}
		msbin[0] |= (ieee[0] << 3);

		return 0;
	}

	// Converte un float da formato IEEE 754 a Microsoft Binary Format (4 byte intermedi)
	int convert_fieeetomsbin(const float src4, float *dest4)
	{
		uint8_t *ieee  = (uint8_t *)&src4;
		uint8_t *msbin = (uint8_t *)dest4;
		uint8_t sign;
		uint8_t msbin_exp;
		int i;

		// Se il numero è 0, il risultato è 0
		if (src4 == 0.0f) {
			for (i = 0; i < 4; i++) msbin[i] = 0;
			return 0;
		}

		msbin_exp = 0;
		sign = ieee[3] & 0x80;
		msbin_exp |= (ieee[3] << 1);
		msbin_exp |= (ieee[2] >> 7);
		
		if (msbin_exp == 0xfe) return 1; // Overflow
		
		msbin_exp += 2; // Bias adjustment: -127 (IEEE) + 128 (MBF) + 1
		for (i = 0; i < 4; i++) msbin[i] = 0;
		
		msbin[3] = msbin_exp;
		msbin[2] |= sign;
		msbin[2] |= (ieee[2] & 0x7f); // Mantissa
		msbin[1] = ieee[1];
		msbin[0] = ieee[0];

		return 0;
	}

} // anonymous namespace

namespace F40Converter 
{
	std::string convertDoubleToMBF_string(double value)
	{
		// C64 KERNAL ROM FIX: The FAC-to-INT conversion routines do not correctly
		// handle -0.0, treating it as a non-zero value. This causes issues in
		// logical operations. We force -0.0 to be treated as +0.0.
		if (value == 0.0) {
			return "$00,$00,$00,$00,$00";
		}

		double temp_dest = 0;
		if (convert_dieeetomsbin(value, &temp_dest) != 0) {
			return "Error: Overflow during conversion.";
		}

		unsigned char *mbf = (unsigned char *)&temp_dest;
		return std::format("${:02X},${:02X},${:02X},${:02X},${:02X}",
						   mbf[7], mbf[6], mbf[5], mbf[4], mbf[3]);
	}

	std::array<unsigned char, 5> convertDoubleToMBF_bytes(double value)
	{
		// C64 KERNAL ROM FIX: The FAC-to-INT conversion routines do not correctly
		// handle -0.0, treating it as a non-zero value. This causes issues in
		// logical operations. We force -0.0 to be treated as +0.0.
		if (value == 0.0) {
			return {0, 0, 0, 0, 0};
		}

		double temp_dest = 0;
		convert_dieeetomsbin(value, &temp_dest);
		unsigned char *mbf = (unsigned char *)&temp_dest;

		return {mbf[7], mbf[6], mbf[5], mbf[4], mbf[3]};
	}

	std::string convertFloatToMBF_string(float value)
	{
		// C64 KERNAL ROM FIX: The FAC-to-INT conversion routines do not correctly
		// handle -0.0, treating it as a non-zero value. This causes issues in
		// logical operations. We force -0.0 to be treated as +0.0.
		if (value == 0.0f) {
			return "$00,$00,$00,$00,$00";
		}

		float temp_dest = 0;
		if (convert_fieeetomsbin(value, &temp_dest) != 0) {
			return "Error: Overflow during conversion.";
		}

		unsigned char* mbf = (unsigned char*)&temp_dest;
		// La conversione da float produce 4 byte significativi. Il 5° byte è 0.
		// Gli indici corretti per un array di 4 byte sono 3, 2, 1, 0.
		return std::format("${:02X},${:02X},${:02X},${:02X},${:02X}",
						   mbf[3], mbf[2], mbf[1], mbf[0], 0x00);
	}

	std::array<unsigned char, 5> convertFloatToMBF_bytes(float value)
	{
		// C64 KERNAL ROM FIX: The FAC-to-INT conversion routines do not correctly
		// handle -0.0, treating it as a non-zero value. This causes issues in
		// logical operations. We force -0.0 to be treated as +0.0.
		if (value == 0.0f) {
			return {0, 0, 0, 0, 0};
		}

		float temp_dest = 0;
		convert_fieeetomsbin(value, &temp_dest);
		unsigned char* mbf = (unsigned char*)&temp_dest;

		return {mbf[3], mbf[2], mbf[1], mbf[0], 0x00};
	}

} // namespace F40Converter
