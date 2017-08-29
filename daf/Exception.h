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
#ifndef DAF_EXCEPTION_H
#define DAF_EXCEPTION_H

#include "OS.h"

#include <stdexcept>
#include <typeinfo>

#define DAF_THROW_EXCEPTION(EXCEPT_TYPE) \
  do { char ss[128];                     \
     DAF_OS::snprintf(ss,sizeof(ss),"%s(%d):%s",typeid(EXCEPT_TYPE).name(),__LINE__,__FILE__); \
     throw EXCEPT_TYPE(ss);              \
  } while (false) /* NOTE No Trailing ';' */

namespace DAF
{
  /**
   \defgroup exception Framework Exceptions

   This group contains exceptions that are used throughout DAF and TAF
   */

 /** \struct InternalException
  * \brief Framework Infrastructure Exception
  *
  *
  * This exception indicates an unexpected operation inside the DAF/TAF framework
  * has occurred at runtime. Typical runtime problems are Illegal states,
  * Thread management issues and resource limitations.
  * \ingroup exception
  */
  struct InternalException : std::runtime_error {
      InternalException(const std::string& msg = typeid(InternalException).name())
          : std::runtime_error(msg) {
      }
  };

 /** \struct InitializationException
  * \brief Indicates an error during initialization
  *
  * This excpetion indicates that an incorrect configuration or initialization
  * process has occurred in the infrastructure or object.
  * \ingroup exception
  */
  struct InitializationException : std::runtime_error {
      InitializationException(const std::string& msg = typeid(InitializationException).name())
          : std::runtime_error(msg) {
      }
  };

 /** \struct TimeoutException
  * \brief Time expiration condition
  *
  * This exception is used in concurrency mechanisms to indicate a time sensitive
  * condition has expired. It is heavily used on poll, attempt concepts
  * \ingroup exception
  */
  struct TimeoutException : std::runtime_error {
    TimeoutException(const std::string& msg = typeid(TimeoutException).name())
    : std::runtime_error(msg)
    {
        DAF_OS::last_error(ETIME);
    }
  };

 /** \struct ClassCastException
  * \brief Class Logic mismatch
  *
  * This exception is used to convey an inconsistency in casting objects.
  * \ingroup exception
  */
  struct ClassCastException : std::logic_error {
      ClassCastException(const std::string& msg = typeid(ClassCastException).name())
          : std::logic_error(msg) {
      }
  };

 /** \struct IndexOutOfRange
  * \brief Specialised std::out_of_range
  *
  * Used in the same way as std::out_of_range in array/vector operations.
  * \ingroup exception
  */
  struct IndexOutOfRange : std::out_of_range {
      IndexOutOfRange(const std::string& msg = typeid(IndexOutOfRange).name())
          : std::out_of_range(msg) {
      }
  };

 /** \struct ObjectNotExistException
  * \brief Represents a missing Object
  *
  * This exception manifests an inconsist state in the handling of objects.
  * This can be in memory managment handling in reference counting techniques,
  * in distributed concepts (similar to CORBA exceptions) to identifying the
  * remote underlying has been removed.
  * \ingroup exception
  */
  struct ObjectNotExistException : std::invalid_argument {
      ObjectNotExistException(const std::string& msg = typeid(ObjectNotExistException).name())
          : std::invalid_argument(msg) {
      }
  };

 /** \struct DateTimeException
  * \brief Invalid Date Time inputs
  *
  * Designates the inputs or current format of a Date Time structure is invalid.
  * \ingroup exception
  */
  struct DateTimeException : std::runtime_error {
      DateTimeException(const std::string& msg = typeid(DateTimeException).name())
          : std::runtime_error(msg) {
      }
  };

 /** \struct IllegalStateException
  * \brief An inconsistent or non-usable state is detected
  *
  * Used internally by the framework to identify when a resource is in an
  * inconsistent or non-usable state.
  * \ingroup exception
  */
  struct IllegalStateException : std::runtime_error {
      IllegalStateException(const std::string& msg = typeid(IllegalStateException).name())
          : std::runtime_error(msg) {
      }
  };

 /** \struct InvocationTargetException
  * \brief Not Used
  *
  * \deprecated
  * Marked for removal.
  * \ingroup exception
  */
  struct InvocationTargetException : std::runtime_error {
      InvocationTargetException(const std::string& msg = typeid(InvocationTargetException).name())
          : std::runtime_error(msg) {
      }
  };

 /** \struct IllegalArgumentException
  * \brief Input argument inconsistency
  *
  * Used when an illegal argument is passed in. This can be a key that doesn't
  * exist in map (see DAF::PropertyManager) or any other class that wants to
  * provide an alternative execution path.
  * \ingroup exception
  */
  struct IllegalArgumentException : std::invalid_argument {
      IllegalArgumentException(const std::string& msg = typeid(IllegalArgumentException).name())
          : std::invalid_argument(msg) {
      }
  };

 /** \struct IllegalLengthException
  * \brief NOT USED
  *
  * \deprecated
  * Marked for removal
  * \ingroup exception
  */
  struct IllegalLengthException : std::length_error {
      IllegalLengthException(const std::string& msg = typeid(IllegalLengthException).name())
          : std::length_error(msg) {
      }
  };

  //------------------- Extended Exceptions ----------------

 /** \struct InterruptedException
  * \brief concurrency execution has been interrupted
  *
  * This exception is used in the DAF concurrency concepts to allow concurrent
  * operations to neatly recover. It is used in when a shared element needs to
  * stipulate it is being removed and pariticipating threads need to be notified.
  * \ingroup exception
  */
  struct InterruptedException : InternalException {
    InterruptedException(const std::string& msg = typeid(InterruptedException).name())
    : InternalException(msg)
    {
        DAF_OS::last_error(EINTR);
    }
  };

  /** \struct LockFailureException
  * \brief Identifies an instance where a lock manipulation has failed
  *
  * This exception is used to identify when an Operating System based lock has failed
  * to be manipulated (locked) A lot of the use cases are around concurrency resources
  * at the OS level.
  * \ingroup exception
  */
  struct LockFailureException : InternalException {
      LockFailureException(const std::string& msg = typeid(LockFailureException).name())
          : InternalException(msg)
      {
          DAF_OS::last_error(ENOLCK);
      }
  };

 /** \struct BrokenBarrierException
  * \brief Concurrency Barrier
  *
  * This exception is used to notify participating threads in a DAF::Barrier
  * that the barrier concept has been broken.
  * \ingroup exception
  */
  struct BrokenBarrierException : InterruptedException {
      BrokenBarrierException(const std::string& msg = typeid(BrokenBarrierException).name())
          : InterruptedException(msg) {
      }
  };

 /** \struct IllegalThreadStateException
  * \brief Inconsistent state in a thread
  *
  * This exception manifests a threads normal execution has been interrupted and
  * entered an illegal state. This could be due to interruption (\sa DAF::InterruptedException)
  * Or high level intervention at the thread management of ACE or DAF.
  * \ingroup exception
  */
  struct IllegalThreadStateException : InternalException {
      IllegalThreadStateException(const std::string& msg = typeid(IllegalThreadStateException).name())
          : InternalException(msg) {
      }
  };

 /** \struct ResourceExhaustionException
  * \brief Identifies an exhaustion of OS resources
  *
  * This exception is used to identify when Operating System resources have been
  * exhausted. Alot of the use cases are around concurrency resources at the OS
  * level.
  * \ingroup exception
  */
  struct ResourceExhaustionException : InternalException {
      ResourceExhaustionException(const std::string& msg = typeid(ResourceExhaustionException).name() )
          : InternalException(msg) {
      }
  };

 /** \struct NotFoundException
  * \brief Specialization on IllegalArgumentException
  *
  * A specialization to distinguish when an element could not be found. Typical
  * use case is a key in a map.
  * \todo MARK for consolidation with IllegalPropertyException, IllegalArgumentException
  * \ingroup exception
  */
  struct NotFoundException : IllegalArgumentException {
      NotFoundException(const std::string& msg = typeid(NotFoundException).name())
          : IllegalArgumentException(msg) {
      }
  };

 /** \struct IllegalPropertyException
  * \brief Related to DAF Properties
  *
  * Identifies that a DAF property is illegal in its value format.
  * \ingroup exception
  */
  struct IllegalPropertyException : IllegalArgumentException {
      IllegalPropertyException(const std::string& msg = typeid(IllegalPropertyException).name())
          : IllegalArgumentException(msg) {
      }
  };

} // namespace DAF

#endif // DAF_EXCEPTION_H
