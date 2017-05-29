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
#ifndef DAF_FUTURERESULT_T_H
#define DAF_FUTURERESULT_T_H

/**
* ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
*
* @file     FutureResult_T.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "Monitor.h"
#include "Runnable.h"

namespace DAF
{
  /**
   * A class maintaining a single reference variable serving as the result
   * of an operation. The result cannot be accessed until it has been set.
   **/

   /** @struct FutureFunctor
   * @brief A class maintaining a single reference variable serving as the result
   * of an operation.
   *
   * The result cannot be accessed until it has been set.
   * \todo{Double check description goes here}
   */
    template <typename T>
    struct FutureFunctor
    {
        /** \todo{Fill this in} */
        virtual ~FutureFunctor(void) {}

        /** \todo{Fill this in} */
        typedef T _value_type;
        /** \todo{Fill this in} */
        typedef T _return_type;

        /** \todo{Fill this in} */
        virtual _return_type operator()(const _value_type& ) = 0;
    };

    template <typename T, typename F = FutureFunctor<T> >

    /** @class FutureResult
    * @brief Brief \todo { fill this in }
    *
    * Details \todo { fill this in }
    *
    * \todo{Double check that the description in the struct shouldn't go here?}
    */
    class FutureResult : protected DAF::Monitor
    {
        volatile bool ready_;

        volatile mutable bool error_;

        volatile mutable size_t waiters_;
        T   value_;

        friend class FutureSetter;

    public:

        /** \todo{Fill this in} */
        typedef T   _value_type;

        /** \todo{Fill this in} */
        typedef F _functor_type;

        /** \todo{Fill this in} */
        FutureResult(void);

        /** \todo{Fill this in} */
        ~FutureResult(void);

        /** \todo{Fill this in} */
        bool        isReady(void) const
        {
            return (this->error_ || this->ready_);
        }

        /** \todo{Fill this in} */
        bool        isError(void) const
        {
            return this->error_;
        }

        /** \todo{Fill this in} */
        const T&    peek(void) const;

        /** \todo{Fill this in} */
        T           get(void) const;


        /// Wait at most msecs to access the reference.
        T           get(time_t msecs) const;

        /// Reset the value and error and set to not-ready, allowing this FutureResult to be reused. This is not particularly recommended and must be done only when you know that no other object is depending on the properties of this FutureResult.
        void        reset(void);


        /// You could even do away with _args_type and with a functor. push the input values into the functor.
        DAF::Runnable_ref operator () (_functor_type &functor, const _value_type &value);

        /// Runnable Setter. Uses Default
        DAF::Runnable_ref operator () (const T &value);

    protected:

        /// Set the error field. This will be reported out as an InvocationTargetException
        virtual void  setError(void);


        /// Set the value, and signal that it is ready. It is not considered an error to set the value more than once, but it is not something you would normally want to do.
        virtual void  setValue(const T&);

    private:

        /**
        * An internal class of FutureResult<T> that encapsulates the running
        * function operation which inturn will generate the result
        **/

        class FutureSetter : public DAF::Runnable
        {
            typedef DAF::FutureResult<T,F>  FResult;
            typedef typename FResult::VFunc FResult_VFunc;

            FResult         &r_;
            FResult_VFunc   fn_;
            void            *args_;

        public:

            FutureSetter(FResult &r, FResult_VFunc fn, void *args)
                : r_(r), fn_(fn), args_(args)
            {
            }

            virtual int run(void);
        };

        class FutureFunctor : public DAF::Runnable
        {
            typedef DAF::FutureResult<T,F>  _future_type;

            _future_type        &r_;
            _functor_type       &func_;
            const _value_type   &value_;

        public:
            FutureFunctor(_future_type &r,
                          _functor_type& func,
                          const _value_type& value
                         )
            : Runnable()
            , r_(r)
            , func_(func)
            , value_(value)
            {
            }

            virtual int run(void);
        };

        struct DefaultSetter : _functor_type
        {
            T operator()(const _value_type& value)
            {
                return value;
            }

        private:
        } _default_setter;
    };
} // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "FutureResult_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("FutureResult_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // DAF_FUTURERESULT_T_H
