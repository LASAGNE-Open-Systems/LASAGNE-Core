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
#ifndef DAF_BITSET_H
#define DAF_BITSET_H

#include "DAF.h"

#include <ace/Auto_Ptr.h>
#include <ace/CDR_Base.h>


namespace DAF
{
    /** \class Bitset
    *\brief  A manipulation utility entity for a sequence of bits of arbitary length
    *
    * This utility entity controls the access and manipulation of a bit sequence
    *         of arbitary length.
    */
    class DAF_Export Bitset
    {

        size_t bits_;

    public:

        typedef unsigned char                       BIT_BYTE_type;

        typedef BIT_BYTE_type *                     BIT_BYTE_ptr;

        typedef ACE_Auto_Array_Ptr<BIT_BYTE_type>   BIT_BYTE_buffer;


        enum {
            BIT_BYTE_size    = sizeof(BIT_BYTE_type), BIT_BYTE_bits = 8 * BIT_BYTE_size
        };

        /**
        * Initializing Constructor
        * \param bits is the capacity
        * \param init_val is the initializing value of the bit sequence
        */
        Bitset(size_t bits = 0, bool init_val = false);

        /**
        * Copy Constructor
        */
        Bitset(const Bitset &);

        virtual ~Bitset(void) {}

        /**
        *  \class bitraits
        *\brief  A traits patterned individual bit manipulation entity
        */
        class DAF_Export bitraits
        {
        public:

            /// Implements &= operation on a particular bit in the bitset sequence
            bitraits & operator &= (bool);
            /// Implements ^= operation on a particular bit in the bitset sequence
            bitraits & operator ^= (bool);
            /// Implements |= operation on a particular bit in the bitset sequence
            bitraits & operator |= (bool);
            /// Implements = operation on a particular bit in the bitset sequence
            bitraits & operator  = (bool);
            /// Implements ~ operation on a particular bit in the bitset sequence
            bitraits & operator  ~ (void);

            // Note Switch Order to stop recursive call

            /// Implements == operation on a particular bit in the bitset sequence
            bool    operator == (bool val) const;
            /// Implements != operation on a particular bit in the bitset sequence
            bool    operator != (bool val) const;
            /// Implements ! operation on a particular bit in the bitset sequence
            bool    operator ! () const;
            /// Implements cast operator access to a part bit in the bitset sequence
            operator bool () const;

        private:

            bitraits(BIT_BYTE_type & bit_byte, const BIT_BYTE_type & bit_mask);

            BIT_BYTE_type & bit_byte_, bit_mask_;

            friend class DAF_Export Bitset;
        };

        /** Accessor to the Bit buffer
        \param off offset in bits to the returned pointer   */
        BIT_BYTE_ptr bit_buffer(size_t off = 0) const;

        /// Pointer to the underlying bit sequence
        operator const void * () const
        {
            return this->bits() ? this->bit_buffer() : 0;
        }

        /** Array Accessor
        \param bit access the number 'bit' in the buffer
        \return value of the bit    */
        bool        operator [] (size_t bit) const;
        /// Access to allow rw access to a particular bit in the underlying bit sequence
        bitraits    operator [] (size_t bit);

        /// Assign a Bitset from another Bitset sequence
        Bitset &    operator  = (const Bitset &bitset);
        /// Append a Bitset onto the end of an existing Bitset sequence
        Bitset &    operator += (const Bitset &bitset);

        /** Compare Bit Buffers
        \param bitset buffer to compare against
        \return  see memcmp*/
        int         compare_to(const Bitset &bitset) const;

        /**
          \name Operators
         These operators limit processing to the common bitset array span [ie min(this->bits,that->bits)]
        */
        ///@{
        Bitset &    operator &= (const Bitset &bitset);
        /// Implement |= operation on corresponding bits in argument sequences with result to the lhs
        Bitset &    operator |= (const Bitset &bitset);
        /// Implement ^= operation on corresponding bits in argument sequences with result to the lhs
        Bitset &    operator ^= (const Bitset &bitset);

        /// Implement == operation on corresponding bits in argument sequences
        bool        operator == (const Bitset &bitset) const;
        /// Implement != operation on corresponding bits in argument sequences
        bool        operator != (const Bitset &bitset) const;
        ///@}

        ///Access to the number of bits in the sequence
        size_t   bits(void) const;
        /// Access to the number of bytes in the sequence NOTE: only bits() are valid
        size_t   size(void) const;

        /** Create a buffer out of subset of bits in this buffer
        \param pos start of new buffer
        \param len end of new buffer
        \return New Bitset class with subset of bits
         */
        Bitset   subits(int pos, int len = EOF) const;

        /** Remove bits from the buffer
        Erase bits starting at a bit position for a maximum bit length; NOTE: remaining bits are left shifted after erase
        \param pos start position of bits for removal
        \param len number of bits to remove from buffer
        */
        Bitset & erase(int pos, int len = EOF);

        /// Invert all the bits in the bitset sequence
        Bitset & flip(void);

        /** Reset n-number of bits to value
        \param bits number of bits to reset value
        \param val the binary value to assign
        */
        Bitset & reset(size_t bits, bool val = false);


    protected:

        /// Reduce bit array to minium needed to hold bits()
        Bitset & trim_bits(void);

    protected:

        /// Underlying bit stream buffer
        BIT_BYTE_buffer bit_buffer_;
    };

    inline size_t
    Bitset::bits(void) const
    {
        return this->bits_;
    }

    inline bool
    Bitset::operator != (const Bitset &bitset) const
    {
        return (*this == bitset) ? false : true;
    }

    inline Bitset operator & (const Bitset &lhs, const Bitset &rhs)
    {
        return Bitset(lhs) &= rhs;
    }

    inline Bitset operator | (const Bitset &lhs, const Bitset &rhs)
    {
        return Bitset(lhs) |= rhs;
    }

    inline Bitset operator ^ (const Bitset &lhs, const Bitset &rhs)
    {
        return Bitset(lhs) ^= rhs;
    }

    inline Bitset operator + (const Bitset &lhs, const Bitset &rhs)
    {
        return Bitset(lhs) += rhs;
    }

    inline Bitset operator ~ (const Bitset &bitset)
    {
        return Bitset(bitset.bits(), true) ^= bitset;
    }

    inline bool operator ! (const Bitset &bitset)
    {
        return Bitset(bitset.bits(), false) == bitset ? true : false;
    }


    inline Bitset::bitraits::bitraits(Bitset::BIT_BYTE_type & bit_byte, const Bitset::BIT_BYTE_type & bit_mask)
        : bit_byte_(bit_byte), bit_mask_(bit_mask)
    {
    }

    inline Bitset::bitraits::operator bool () const
    {
        return (this->bit_byte_ & this->bit_mask_) ? true : false;
    }

    inline bool Bitset::bitraits::operator == (bool val) const
    {
        return (val == *this);  // Note Switch Order to stop recursive call
    }

    inline bool Bitset::bitraits::operator != (bool val) const
    {
        return !(*this == val);
    }

    inline bool Bitset::bitraits::operator ! () const
    {
        return (*this == false);
    }

} // namespace DAF

DAF_Export std::ostream & operator << (std::ostream &, const DAF::Bitset &);

#endif
