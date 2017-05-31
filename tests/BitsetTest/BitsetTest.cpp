/***************************************************************
    Copyright 2016, 2017 Defence Science and Technology Group,
    Department of Defence,
    Australian Government

	This file is part of LASAGNE.

    LASAGNE is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    LASAGNE is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with LASAGNE.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************/
#include "daf/Bitset.h"

#include <iostream>
#include <typeinfo>

#define MIN_BITS    6
#define MAX_BITS    33

int main(int argc, char *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);

    DAF::Bitset bitset;

    std::cout << "Testing default[0] ";
    try {
        bitset[0] = true; ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed Range Check on Default\n")), -1);
    } catch (const std::out_of_range&) {
        if (bitset.bits()) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed Range Check on Default\n")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing reset/init ";
    const unsigned char BUF_9_BITS_TRUE[2] = { 0xFF, 0x80 };
    {
        bitset.reset(9, true);

        if (bitset.size() != sizeof(BUF_9_BITS_TRUE) || DAF_OS::memcmp(bitset.bit_buffer(), BUF_9_BITS_TRUE, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to Initialize Buffer Correctly%s")
                , DAF::bit_dump(bitset, bitset.bits()).c_str()), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing copy cntr() ";
    const DAF::Bitset::BIT_BYTE_type BUF_MIN_BITS_TRUE[1] = { 0xFC };
    {
        DAF::Bitset bitset_cpy(bitset.reset(MIN_BITS, true));

        do {
            if (bitset.size() == sizeof(BUF_MIN_BITS_TRUE) && bitset_cpy.size() == sizeof(BUF_MIN_BITS_TRUE)) {
                if (DAF_OS::memcmp(bitset.bit_buffer(), BUF_MIN_BITS_TRUE, bitset.size()) == 0) {
                    if (DAF_OS::memcmp(bitset_cpy.bit_buffer(), BUF_MIN_BITS_TRUE, bitset_cpy.size()) == 0) {
                        break;
                    }
                }
            }
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to Initialize Buffer from Copy%s")
                , DAF::bit_dump(bitset_cpy, bitset_cpy.bits()).c_str()), -1);
        } while (false);
    }
    std::cout << "\tOK!" << std::endl;

    const DAF::Bitset::BIT_BYTE_type BUF_MAX_BITS_TRUE[5] = { 0xFF, 0xFF, 0xFF, 0xFF, 0x80 };
    const DAF::Bitset::BIT_BYTE_type BUF_MAX_BITS_ZERO[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };

    bitset.reset(MAX_BITS, true);

    std::cout << "Testing subits(10,22) ";
    const DAF::Bitset::BIT_BYTE_type BUF_SUBITS_SET[3] = { 0xFF, 0xFF, 0xFC };
    {
        DAF::Bitset subits(bitset.subits(10, 22));
        if (subits.size() != sizeof(BUF_SUBITS_SET)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed subits with incorrect length %d\n")
                , int(subits.bits())), -1);
        }
        else if (DAF_OS::memcmp(subits, BUF_SUBITS_SET, subits.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed subits with wrong bits set%s")
                , DAF::bit_dump(subits, subits.bits()).c_str()), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    const DAF::Bitset::BIT_BYTE_type BUF_ERASE_SET[4] = { 0xFF, 0xFF, 0xFF, 0x80 };
    {
        std::cout << "Testing erase(21,8) ";
        DAF::Bitset erasebits(bitset); erasebits.erase(21, 8);
        if (erasebits.size() != sizeof(BUF_ERASE_SET)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed erase with incorrect length %d\n")
                , int(erasebits.bits())), -1);
        }
        else if (DAF_OS::memcmp(erasebits, BUF_ERASE_SET, erasebits.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed erase with wrong bits set%s")
                , DAF::bit_dump(erasebits, erasebits.bits()).c_str()), -1);
        }
        std::cout << "\tOK!" << std::endl << "Testing erase(24)->END ";
        erasebits.erase(24);  // Erase to end
        if (erasebits.size() != sizeof(BUF_ERASE_SET) - 1) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed erase with incorrect length %d\n")
                , int(erasebits.bits())), -1);
        }
        else if (DAF_OS::memcmp(erasebits, BUF_ERASE_SET, erasebits.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed erase with wrong bits set%s")
                , DAF::bit_dump(erasebits, erasebits.bits()).c_str()), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    DAF::Bitset boolbits(bitset);

    std::cout << "Testing ~Bitset ";
    {
        DAF::Bitset bINV(~boolbits);
        if (bINV.size() != sizeof(BUF_MAX_BITS_ZERO)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed ~ with incorrect length %d\n")
                , int(bINV.bits())), -1);
        }
        else if (DAF_OS::memcmp(bINV, BUF_MAX_BITS_ZERO, bINV.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed ~ with wrong bits set%s")
                , DAF::bit_dump(bINV, bINV.bits()).c_str()), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing &= Bitset ";
    {
        boolbits &= bitset;

        if (boolbits.size() != sizeof(BUF_MAX_BITS_TRUE)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed &= with incorrect length\n")), -1);
        }
        else if (DAF_OS::memcmp(boolbits, BUF_MAX_BITS_TRUE, boolbits.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed &= with wrong bits set%s")
                , DAF::bit_dump(boolbits, boolbits.bits()).c_str()), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing ^= Bitset ";
    {
        boolbits ^= bitset;

        if (boolbits.size() != sizeof(BUF_MAX_BITS_ZERO)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed ^= with incorrect length\n")), -1);
        }
        else if (DAF_OS::memcmp(boolbits, BUF_MAX_BITS_ZERO, boolbits.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed ^= with wrong bits set\n")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing |= Bitset ";
    {
        boolbits |= bitset;

        if (boolbits.size() != sizeof(BUF_MAX_BITS_TRUE)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed |= with incorrect length\n")), -1);
        }
        else if (DAF_OS::memcmp(boolbits, BUF_MAX_BITS_TRUE, boolbits.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed |= with wrong bits set\n")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing == Bitset ";
    {
        if (bitset.size() != sizeof(BUF_MAX_BITS_TRUE) || DAF_OS::memcmp(bitset.bit_buffer(), BUF_MAX_BITS_TRUE, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to Initialize Buffer Correctly")), -1);
        }
        else if (bitset != DAF::Bitset(MAX_BITS - 1, true)) {  // Only checks overlapping bits
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed in Differing Length ")), -1);
        }
        else {
            DAF::Bitset test(MAX_BITS, true); if (test[1] = false) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed in set Value ")), -1);
            }
            else if (test == bitset) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed with Differing Data ")), -1);
            }
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing != Bitset ";
    {
        if (bitset != DAF::Bitset(MAX_BITS, true)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed with Same Bits ")), -1);
        }
        else {
            DAF::Bitset test(MAX_BITS, true); if (test[1] = false) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed in set Value ")), -1);
            }
            else if (test != bitset ? false : true) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed with different Bits ")), -1);
            }
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing  + Bitset ";
    const DAF::Bitset::BIT_BYTE_type BUF_MAX_ADD_SET[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0 };
    {
        DAF::Bitset b_add(bitset + DAF::Bitset(9, true));
        if (b_add.size() != sizeof(BUF_MAX_ADD_SET)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed in '+' to increase buffer size\n")), -1);
        }
        else if (DAF_OS::memcmp(b_add, BUF_MAX_ADD_SET, b_add.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed '+' with wrong Bits\n")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing  = operator ";
    const DAF::Bitset::BIT_BYTE_type BUF_MAX_BITS_SET[5] = { 0xBF, 0xFF, 0xFF, 0xFF, 0x80 };
    {
        if ((bitset[1] = false) ? true : false)  {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to set false\n")), -1);
        }
        else if (DAF_OS::memcmp(bitset, BUF_MAX_BITS_SET, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed with different Bits\n")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing  ! operator ";
    {
        if (!bitset[1] ? false : true) { // Bit 0 is true HERE!!
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed !true != false\n")), -1);
        }
        else if ((bitset[1] = !bitset[1]) ? DAF_OS::memcmp(bitset.bit_buffer(), BUF_MAX_BITS_TRUE, bitset.size()) : true) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to set true\n")), -1);
        }
        else if (!(bitset[1] = false) ? false : true)  {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed !true != false\n")), -1);
        }
        else if (DAF_OS::memcmp(bitset, BUF_MAX_BITS_SET, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed in ! operator buffer state\n")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing == operator ";
    {
        if (bitset[1] == false ? false : true)  {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to test for == false\n")), -1);
        }
        else bitset[1] = true;
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing != operator ";
    {
        if (bitset[1] != true ? true : bitset[1] = false)  {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to test for != true\n")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing ^= operator ";
    {
        if (DAF_OS::memcmp(bitset, BUF_MAX_BITS_SET, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to initialize to false\n")), -1);
        }
        else if ((bitset[1] ^= true) ? DAF_OS::memcmp(bitset, BUF_MAX_BITS_TRUE, bitset.size()) : true) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (false ^ true) != true")), -1);
        }
        else if ((bitset[1] ^= false) ? DAF_OS::memcmp(bitset, BUF_MAX_BITS_TRUE, bitset.size()) : true) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (true ^ false) != true")), -1);
        }
        else if ((bitset[1] ^= true) || DAF_OS::memcmp(bitset, BUF_MAX_BITS_SET, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (true ^ true)  != false")), -1);
        }
        else if ((bitset[1] ^= false) || DAF_OS::memcmp(bitset, BUF_MAX_BITS_SET, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (false ^ false) != false")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing |= operator ";
    {
        if (bitset[1] |= false) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (false | false) != false")), -1);
        }
        else if ((bitset[1] |= true) ? DAF_OS::memcmp(bitset, BUF_MAX_BITS_TRUE, bitset.size()) : true) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (false | true) != true")), -1);
        }
        else if ((bitset[1] |= false) ? DAF_OS::memcmp(bitset, BUF_MAX_BITS_TRUE, bitset.size()) : true) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (true | false) != true")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing &= operator ";
    {
        if ((bitset[1] &= true) ? DAF_OS::memcmp(bitset, BUF_MAX_BITS_TRUE, bitset.size()) : false) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (true & true)   != true")), -1);
        }
        else if ((bitset[1] &= false) || DAF_OS::memcmp(bitset, BUF_MAX_BITS_SET, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (true & false)  != false")), -1);
        }
        else if ((bitset[1] &= false) || DAF_OS::memcmp(bitset, BUF_MAX_BITS_SET, bitset.size())) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed (false & false) != false")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    std::cout << "Testing out_of_range ";
    {
        if ((bitset[32] ^= true) ? false : (bitset[32] ^= true)) try {
            if (bitset[33]) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed Range Check\n")), -1);
            }
        }
        catch (const std::out_of_range&) {
            if (DAF_OS::memcmp(bitset, BUF_MAX_BITS_SET, sizeof(BUF_MAX_BITS_SET))) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed in Range Check with resultant buffer state\n")), -1);
            }
        }
        else {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("Failed to set Last Bit to false")), -1);
        }
    }
    std::cout << "\tOK!" << std::endl;

    return 0;
}
