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
#define DAF_BITSET_CPP

#include "Bitset.h"

#include <iostream>

DAF_Export std::ostream & operator << (std::ostream & os, const DAF::Bitset & bs)
{
    return os << DAF::bit_dump(bs, bs.bits());
}

namespace {

    size_t  bit_bytes(size_t bits)
    {
        return size_t((bits / 8) + size_t((bits % 8) ? 1 : 0));
    }

    const unsigned char & bit_mask(size_t bit)
    {
        static const unsigned char _bit_mask[8] =
        {
            0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
        };

        return _bit_mask[bit % 8];
    }

}

namespace DAF
{
    Bitset::Bitset(size_t bits, bool val)
        : bits_(bits), bit_buffer_(0)
    {
        if (ace_range(0, INT_MAX, int(bits)) == int(bits)) {
            size_t len = this->size();
            if (len) {
                BIT_BYTE_ptr p(0); ACE_NEW_NORETURN(p, BIT_BYTE_type[len]);
                this->bit_buffer_.reset(p); if (p == 0) {
                    DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException);
                }
                DAF_OS::memset(p, (val ? -1 : 0), len);

                if (val) this->trim_bits();
            }
            return;
        }

        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);
    }

    Bitset::Bitset(const Bitset &bitset)
        : bits_(bitset.bits()), bit_buffer_(0)
    {
        size_t len = this->size();
        if (len) {
            BIT_BYTE_ptr p(0); ACE_NEW_NORETURN(p, BIT_BYTE_type[len]);
            this->bit_buffer_.reset(p); if (p == 0) {
                DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException);
            }
            DAF_OS::memcpy(p, bitset, len);
        }
    }

    Bitset::BIT_BYTE_ptr
    Bitset::bit_buffer(size_t off) const
    {
        if (this->bits()) for (Bitset::BIT_BYTE_ptr p = this->bit_buffer_.get(); p;) {

            if (off >= this->size()) {
                DAF_THROW_EXCEPTION(DAF::IndexOutOfRange);
            }
            return p + off;

        } else DAF_THROW_EXCEPTION(DAF::IndexOutOfRange);

        return 0;  // returns 0 when no buffer or bits.
    }

    size_t
    Bitset::size(void) const
    {
        return bit_bytes(this->bits());
    }

    bool
    Bitset::operator == (const Bitset &bitset) const
    {
        if (this->compare_to(bitset)) { // Short curcuit equivalence

            for (int i, bit = int(ace_min(this->bits(), bitset.bits())); bit;) {
                if (bit % BIT_BYTE_bits && bit--) {
                    if ((*this)[bit] != bitset[bit]) {
                        return false;
                    }
                }
                else if ((i = (bit / BIT_BYTE_bits)) > 0) {
                    return DAF_OS::memcmp(*this, bitset, size_t(i)) == 0;
                }
                else break; // Should never happen
            }
        }

        return true;
    }

    Bitset &
    Bitset::operator = (const Bitset &bitset)
    {
        if (this != &bitset) do { // Doing it to ourselves?

            size_t len = bit_bytes(this->bits_ = bitset.bits());

            if (len) {
                BIT_BYTE_ptr p(0); ACE_NEW_NORETURN(p, BIT_BYTE_type[len]);
                this->bit_buffer_.reset(p); if (p == 0) {
                    DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException);
                }
                DAF_OS::memcpy(p, bitset, len); break;
            }

            this->bit_buffer_.reset(0);

        } while (false);

        return *this;
    }

    Bitset &
    Bitset::operator += (const Bitset &bitset)
    {
        size_t pos = this->bits();

        if (bitset) {  // Does the addition have any bits?

            size_t bits = pos + bitset.bits(); // Work out how many bits total.

            if (ace_range(0, INT_MAX, int(bits)) != int(bits)) {
                DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);
            }

            for (size_t len = bit_bytes(bits); len > this->size();) {
                BIT_BYTE_ptr p(0); ACE_NEW_NORETURN(p, BIT_BYTE_type[len]); if (p == 0) {
                    DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException);
                }
                DAF_OS::memset(p, 0, len); if (pos) {
                    DAF_OS::memcpy(p, *this, this->size());
                }
                this->bit_buffer_.reset(p); break;
            }

            this->bits_ = bits;  // Update Our Length

        } else return *this; // Adding nothing

        for (size_t i = 0, bits = bitset.bits(); bits--; i++) {
            (*this)[pos++] = bitset[i];
        }

        return this->trim_bits();
    }

    Bitset &
    Bitset::operator &= (const Bitset &bitset)
    {
        if (this != &bitset) { // Doing it to ourselves?

            for (int i, bit = int(ace_min(this->bits(), bitset.bits())); bit;) {
                if (bit % BIT_BYTE_bits && bit--) {
                    (*this)[bit] &= bitset[bit];
                }
                else if ((i = (bit / BIT_BYTE_bits)) > 0 && i--) {
                    *this->bit_buffer(i) &= *bitset.bit_buffer(i); bit -= BIT_BYTE_bits;
                }
                else break; // Should never happen
            }
        }

        return *this;
    }

    Bitset &
    Bitset::operator |= (const Bitset &bitset)
    {
        if (this != &bitset) { // Doing it to ourselves?

            for (int i, bit = int(ace_min(this->bits(), bitset.bits())); bit;) {
                if (bit % BIT_BYTE_bits && bit--) {
                    (*this)[bit] |= bitset[bit];
                }
                else if ((i = (bit / BIT_BYTE_bits)) > 0 && i--) {
                    *this->bit_buffer(i) |= *bitset.bit_buffer(i); bit -= BIT_BYTE_bits;
                }
                else break; // Should never happen
            }
        }

        return *this;
    }

    Bitset &
    Bitset::operator ^= (const Bitset &bitset)
    {
        if (this != &bitset) { // Doing it to ourselves?

            for (int i, bit = int(ace_min(this->bits(), bitset.bits())); bit;) {
                if (bit % BIT_BYTE_bits && bit--) {
                    (*this)[bit] ^= bitset[bit];
                }
                else if ((i = (bit / BIT_BYTE_bits)) > 0 && i--) {
                    *this->bit_buffer(i) ^= *bitset.bit_buffer(i); bit -= BIT_BYTE_bits;
                }
                else break; // Should never happen
            }
        }

        return *this;
    }

    int
    Bitset::compare_to(const Bitset &bitset) const
    {
        if (this == &bitset) { // Doing it to ourselves?
            return 0; // ourselves
        }

        size_t this_bits = this->bits(), that_bits = bitset.bits();

        if (this_bits == that_bits) {
            for (size_t len = this->size(); len;) {
                return DAF_OS::memcmp(*this, bitset, len);
            }
        }

        return int(this_bits - that_bits);
    }

    Bitset
    Bitset::subits(int pos, int len) const
    {
        if (*this && ace_range(0, int(this->bits() - 1), pos) == pos) {
            size_t bits = ace_min(this->bits() - pos, size_t(len));
            Bitset bitset(bits, false); for (int i = 0; bits--;) {
                bitset[i++] = bool((*this)[pos++]);
            }
            return bitset;
        }
        DAF_THROW_EXCEPTION(DAF::IndexOutOfRange);
    }

    Bitset &
    Bitset::erase(int pos, int len)
    {
        if (*this && ace_range(0, int(this->bits() - 1), pos) == pos) {
            size_t bits = ace_min(this->bits() - pos, size_t(len));
            for (int epos = int(pos + bits); this->bits() > size_t(epos);) {
                (*this)[pos++] = bool((*this)[epos++]);
            }
            this->bits_ = pos; return this->trim_bits();
        }
        DAF_THROW_EXCEPTION(DAF::IndexOutOfRange);
    }

    Bitset &
    Bitset::flip(void)
    {
        for (size_t i = this->size(); i--;) {
            this->bit_buffer_[int(i)] ^= BIT_BYTE_type(-1);
        }
        return this->trim_bits();
    }

    Bitset::bitraits
    Bitset::operator [] (size_t bit)
    {
        if (*this && ace_range(size_t(0), this->bits() - 1, bit) == bit) {
            return bitraits(this->bit_buffer_[int(bit / BIT_BYTE_bits)], bit_mask(bit));
        }
        DAF_THROW_EXCEPTION(DAF::IndexOutOfRange);
    }

    bool
    Bitset::operator [] (size_t bit) const
    {
        if (*this && ace_range(size_t(0), this->bits() - 1, bit) == bit) {
            return (this->bit_buffer_[int(bit / BIT_BYTE_bits)] & bit_mask(bit)) ? true : false;
        }
        DAF_THROW_EXCEPTION(DAF::IndexOutOfRange);
    }

    Bitset &
    Bitset::reset(size_t bits, bool val)
    {
        if (ace_range(0, INT_MAX, int(bits)) == int(bits)) {
            size_t len = bit_bytes(bits); this->bits_ = 0; // Clear out existing
            if (len) {
                BIT_BYTE_ptr p(0); ACE_NEW_NORETURN(p, BIT_BYTE_type[len]);
                this->bit_buffer_.reset(p); if (p == 0) {
                    DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException);
                }
                DAF_OS::memset(p, (val ? -1 : 0), len); this->bits_ = bits;

                if (val) this->trim_bits();

            } else this->bit_buffer_.reset(0);

            return *this;
        }

        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);
    }

    Bitset &
    Bitset::trim_bits(void)
    {
        size_t bits = this->bits(); if (bits % BIT_BYTE_bits) {
            // Clear remaining Bits if initially not set to zero
            BIT_BYTE_type block_mask(0); // Build Block Mask
            for (int bit = (bits % BIT_BYTE_bits); bit--;) {
                block_mask |= bit_mask(bit);
            }
            this->bit_buffer_[int(bits / BIT_BYTE_bits)] &= block_mask;
        }
        return *this;
    }

    /**************************************************************************/

    Bitset::bitraits &
    Bitset::bitraits::operator &= (bool val)
    {
        if (!val) {
            this->bit_byte_ &= (~this->bit_mask_);
        }
        return *this;
    }

    Bitset::bitraits &
    Bitset::bitraits::operator |= (bool val)
    {
        if (val) {
            this->bit_byte_ |= this->bit_mask_;
        }
        return *this;
    }

    Bitset::bitraits &
    Bitset::bitraits::operator ^= (bool val)  // Validate
    {
        if (val) {
            this->bit_byte_ ^= this->bit_mask_;
        } else if (this->bit_byte_ & this->bit_mask_) {
            ; // true ^ false == true;
        } else { // false ^ false == false
            this->bit_byte_ &= ~this->bit_mask_;
        }
        return *this;
    }

    Bitset::bitraits &
    Bitset::bitraits::operator = (bool val)
    {
        if (val) {
            this->bit_byte_ |= this->bit_mask_;
        } else {
            this->bit_byte_ &= ~this->bit_mask_;
        }
        return *this;
    }

    Bitset::bitraits &
    Bitset::bitraits::operator ~ (void)
    {
        this->bit_byte_ ^= this->bit_mask_; return *this;
    }

} // namespace DAF
