/*********************************************************
*   Copyright (C) 2017 All rights reserved.
*   
* File Name: SpinLock.h
* Purpose:
* Creation Date: 2017-10-20
* Created By: bxq2011hust@qq.com
* Compile :
*********************************************************/

#ifndef SPIN_LOCK_H_
#define SPIN_LOCK_H_

#include <atomic>
namespace dev
{

class SpinLock
{
    std::atomic<bool> flag = ATOMIC_VAR_INIT(false);

  public:
    SpinLock() = default;
    SpinLock(const SpinLock &) = delete;
    SpinLock &operator=(const SpinLock &) = delete;
    void lock()
    {
        bool expected = false;
        while (!flag.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
            expected = false;
    }
    void unlock()
    {
        flag.store(false, std::memory_order_release);
    }
};
}
#endif //SPIN_LOCK_H_
