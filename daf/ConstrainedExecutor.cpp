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
#define DAF_CONSTRAINEDEXECUTOR_CPP

#include "ConstrainedExecutor.h"
#include "DirectExecutor.h"

namespace {

    class ConstraintRunnable : public DAF::Runnable
    {
        DAF::Runnable_ref   command_;
        DAF::Semaphore      &semaphore_;

    public:

        ConstraintRunnable(const DAF::Runnable_ref &command, DAF::Semaphore &semaphore)
            : command_  (command)
            , semaphore_(semaphore)
        {
            this->semaphore_.acquire();
        }

        virtual int run(void)
        {
            int rtn_val = 0;
            if (this->command_) try {
                rtn_val = DAF::DirectExecutor().execute(this->command_._retn());
            } DAF_CATCH_ALL_ACTION(this->semaphore_.release()) {}

            this->semaphore_.release(); return rtn_val;
        }

        virtual long runPriority(void) const
        {
            return this->command_->runPriority();
        }
    };
}

namespace DAF
{
    ConstrainedExecutor::ConstrainedExecutor(DAF::Executor &executor, size_t permits)
        : semaphore_(ace_range(1, ACE_Numeric_Limits<int>::max(), int(permits)))
        , executor_ (executor)
    {
        this->permits_ = this->semaphore_.permits();
    }

    int
    ConstrainedExecutor::execute(const Runnable_ref &command)
    {
        return this->executor_.execute(new ConstraintRunnable(command, this->semaphore_));
    }

    size_t
    ConstrainedExecutor::available(void) const
    {
        return size_t(this->semaphore_.permits());
    }

    size_t
    ConstrainedExecutor::size(void) const
    {
        return size_t(this->permits_ - this->available());
    }

} // namespace DAF
