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
#ifndef DAF_ARGV_H
#define DAF_ARGV_H

#include "DAF_export.h"

#include <ace/ARGV.h>

#include <string>
#include <list>

/** @namespace DAF */
namespace DAF
{
    /**
    * @class ARGV
    *
    * @brief Builds a counted argument vector (ala argc/argv) from either
    * a string or a set of separate tokens.
    *
    * This class preserves whitespace
    * within tokens only if the whitespace-containing token is enclosed in
    * either single (') or double (") quotes. This is consistent with the
    * expected behavior if an argument vector obtained using this class is
    * passed to, for example, ACE_Get_Opt.
    *
    * This class can substitute environment variable values for tokens that
    * are environment variable references (e.g., @c $VAR). This only works
    * if the token is an environment variable reference and nothing else; it
    * doesn't substitute environment variable references within a token.
    * For example, @c $HOME/file will not substitute the value of the HOME
    * environment variable.
    */
    class DAF_Export ARGV
    {
        mutable ACE_ARGV args_;

    public:

        /**
        * Initialize this object so arguments can be added later using one
        * of the add methods. This is referred to as the @i iterative method
        * of adding arguments to this object.
        */
        ARGV(bool substitute_env_args = true);

        /**
        * Copy aguments to backing list variant
        *
        * @param argv  An array of tokens to initialize the object with. All needed
        *              data is copied from @a argv during this call; the pointers
        *              in @a argv are not needed after this call, and the memory
        *              referred to by @a argv is not referenced by this object.
        *
        * @param substitute_env_args  If non-zero, any element of @a argv that is
        *              an environment variable reference (e.g., @c $VAR) will have
        *              its environment variable value in the resultant vector
        *              in place of the environment variable name.
        *
        * NOTE: arguments may need to be enclosed in matching paired quotes ('"' or '\'').
        */
        ARGV(const ACE_ARGV &argv, bool substitute_env_args = true);

        /**
        * Initializes the argument vector from a set of arguments. Any environment
        * variable references are translated (if applicable) during execution of
        * this method. In contrast with ACE_ARGV_T(CHAR_TYPE *[], bool, bool), this
        * ctor does not require argv to be 0-terminated as the number of arguments
        * is provided explicitely.
        *
        * @param argc  The number of arguments in the argv array.
        *
        * @param argv  An array of tokens to initialize the object with. All needed
        *              data is copied from @a argv during this call; the pointers
        *              in @a argv are not needed after this call, and the memory
        *              referred to by @a argv is not referenced by this object.
        *
        * @param substitute_env_args  If non-zero, any element of @a argv that is
        *              an environment variable reference (e.g., @c $VAR) will have
        *              its environment variable value in the resultant vector
        *              in place of the environment variable name.
        *
        * @param quote_args  If non-zero each argument @a argv[i] needs to
        *                    be enclosed in double quotes ('"').
        */
        ARGV(int argc, char *argv[], bool substitute_env_args = true, bool quote_args = false);

        /**
        * Initializes the argument vector from a set of arguments. Any environment
        * variable references are translated (if applicable) during execution of
        * this method.
        *
        * @param argv  An array of tokens to initialize the object with. The
        *              array must be terminated with a 0 pointer. All needed
        *              data is copied from @a argv during this call; the pointers
        *              in @a argv are not needed after this call, and the memory
        *              referred to by @a argv is not referenced by this object.
        *
        * @param substitute_env_args  If non-zero, any element of @a argv that is
        *              an environment variable reference (e.g., @c $VAR) will have
        *              its environment variable value in the resultant vector
        *              in place of the environment variable name.
        *
        * @param quote_args  If non-zero each argument @a argv[i] needs to
        *                    be enclosed in double quotes ('"').
        */
        ARGV(const char *argv[], bool substitute_env_args = true, bool quote_args = false);

        /**
        * Splits the specified string into an argument vector. Arguments in the
        * string are delimited by whitespace. Whitespace-containing arguments
        * must be enclosed in quotes, either single (') or double (").
        *
        * @param buf   A nul-terminated CHAR_TYPE array to split into arguments
        *              for the vector.
        *
        * @param substitute_env_args  If non-zero, any token that is an
        *              environment variable reference (e.g., @c $VAR) will have
        *              its environment variable value in the resultant vector
        *              in place of the environment variable name.
        */
        ARGV(const char args[], bool substitute_env_args = true);

        /** @name Accessor methods
        *
        * These methods access the argument vector contained in this object.
        */
        //@{
        /**
        * Returns the specified element of the current argument vector.
        *
        * @param index   Index to the desired element.
        *
        * @retval Pointer to the indexed string.
        * @retval 0 if @a index is out of bounds.
        */
        const char * operator [] (int index) const;

        /** \todo{Fill this in} */
        operator const ACE_ARGV & () const;

        /**
        * Returns the current argument vector. The returned pointers are to data
        * maintained internally to this class. Do not change or delete either the
        * pointers or the memory to which they refer.
        */
        char **argv(void);

        /// Returns the current number of arguments.
        int argc(void) const;

        /**
        * Add another argument.
        *
        * @note This method copies the specified pointer, but not the data
        *       contained in the referenced memory. Thus, if the content of
        *       the memory referred to by @a next_arg are changed after this
        *       method returns, the results are undefined.
        *
        * @param args        Pointer to the next argument to add to the vector.
        *
        * @param quote_arg   The argument @a next_arg need to be quoted while
        *                    adding to the vector.
        *
        * @retval 0 on success; -1 on failure. Most likely @c errno values are:
        *       - EINVAL: This object is not in iterative mode.
        *       - ENOMEM: Not enough memory available to save @a next_arg.
        */
        int add(const char args[], bool quote_arg = false);

        /**
        * Add an array of arguments.
        *
        * @note This method copies the specified pointers, but not the data
        *       contained in the referenced memory. Thus, if the content of
        *       the memory referred to by any of the @a argv elements is
        *       changed after this method returns, the results are undefined.
        *
        * @param args    Pointers to the arguments to add to the vector.
        *                @a args must be terminated by a 0 pointer.
        *
        * @param quote_args  If non-zero each argument @a argv[i] needs to
        *                    be enclosed in double quotes ('"').
        *
        * @retval 0 on success; -1 on failure. Most likely @c errno values are:
        *       - EINVAL: This object is not in iterative mode.
        *       - ENOMEM: Not enough memory available to save @a next_arg.
        */
        int add(const char *args[], bool quote_args = false);

        /** \todo{Fill this in} */
        bool substitute_env_args(void) const
        {
            return this->substitute_env_args_;
        }

    private:

        int push_argv(const ACE_ARGV &argv);

        bool substitute_env_args_;

        std::list<std::string> args_list_;

    private:

        // = Prevent assignment and initialization.
        ACE_UNIMPLEMENTED_FUNC(void operator = (const ARGV &))
        ACE_UNIMPLEMENTED_FUNC(ARGV(const ARGV &))
    };

    inline ARGV::ARGV(bool substitute_env_args) : args_(substitute_env_args)
        , substitute_env_args_(substitute_env_args)
    {
    }

    inline ARGV::ARGV(const ACE_ARGV &argv, bool substitute_env_args) : args_(substitute_env_args)
        , substitute_env_args_(substitute_env_args)
    {
        this->push_argv(argv);
    }

    inline ARGV::ARGV(int argc, char *argv[], bool substitute_env_args, bool quote_args) : args_(substitute_env_args)
        , substitute_env_args_(substitute_env_args)
    {
        this->push_argv(ACE_ARGV(argc, argv, substitute_env_args, quote_args));
    }

    inline ARGV::ARGV(const char *argv[], bool substitute_env_args, bool quote_args) : args_(substitute_env_args)
        , substitute_env_args_(substitute_env_args)
    {
        this->push_argv(ACE_ARGV(const_cast<char**>(argv), substitute_env_args, quote_args));
    }

    inline ARGV::ARGV(const char args[], bool substitute_env_args) : args_(substitute_env_args)
        , substitute_env_args_(substitute_env_args)
    {
        this->push_argv(ACE_ARGV(args, substitute_env_args));
    }

    inline const char *
    ARGV::operator [] (int index) const
    {
        return this->args_[index];
    }

    inline
    ARGV::operator const ACE_ARGV & () const
    {
        return this->args_;
    }

    inline char **
    ARGV::argv(void)
    {
        return this->args_.argv();
    }

    /// Returns the current number of arguments.
    inline int
    ARGV::argc(void) const
    {
        return this->args_.argc();
    }

}   // namespace DAF

typedef class DAF::ARGV     DAF_ARGV;

#endif // DAF_ARGV_H
