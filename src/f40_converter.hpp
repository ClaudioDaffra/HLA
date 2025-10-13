
// *********
// f40_converter.hpp
// *********

#pragma once

#include <string>
#include <array>
#include <cstdint>

namespace F40Converter 
{
    /**
     * @brief Converte un double (64-bit) nel formato Microsoft Binary Format (5 byte) e lo restituisce come stringa.
     * Esempio di output: "$82,$49,$0f,$da,$a2"
     * @param value Il valore double da convertire.
     * @return Una std::string con la rappresentazione dei 5 byte.
     */
    std::string convertDoubleToMBF_string(double value);

    /**
     * @brief Converte un double (64-bit) nel formato Microsoft Binary Format e restituisce i 5 byte.
     * @param value Il valore double da convertire.
     * @return Un std::array contenente i 5 byte del formato MBF.
     */
    std::array<unsigned char, 5> convertDoubleToMBF_bytes(double value);

    /**
     * @brief Converte un float (32-bit) nel formato Microsoft Binary Format (5 byte) e lo restituisce come stringa.
     * @param value Il valore float da convertire.
     * @return Una std::string con la rappresentazione dei 5 byte.
     */
    std::string convertFloatToMBF_string(float value);

    /**
     * @brief Converte un float (32-bit) nel formato Microsoft Binary Format e restituisce i 5 byte.
     * @param value Il valore float da convertire.
     * @return Un std::array contenente i 5 byte del formato MBF.
     */
    std::array<unsigned char, 5> convertFloatToMBF_bytes(float value);

} // namespace F40Converter

// end f40_converter.hpp

