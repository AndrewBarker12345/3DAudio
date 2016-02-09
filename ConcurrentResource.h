/*
     3DAudio: simulates surround sound audio for headphones
     Copyright (C) 2016  Andrew Barker
     
     This program is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.
     
     The author can be contacted via email at andrew.barker.12345@gmail.com.
 */

#ifndef ConcurrentResource_h
#define ConcurrentResource_h

#include <array>
#include <mutex>
#include <thread>

// a sufficiently capable mutex
// see: http://stackoverflow.com/questions/21892934/how-to-assert-if-a-stdmutex-is-lockedclass
class Mutex : public std::recursive_mutex
{
public:
    void lock()
    {
        std::recursive_mutex::lock();
        holder = std::this_thread::get_id();
        ++count;
    }
    bool try_lock() noexcept
    {
        if (std::recursive_mutex::try_lock()) {
            holder = std::this_thread::get_id();
            ++count;
            return true;
        }
        return false;
    }
    void unlock() noexcept
    {
        if (--count == 0)
            holder = std::thread::id();
        std::recursive_mutex::unlock();
    }
    bool isLocked() const noexcept
    {
        return holder != std::thread::id();
    }
    bool isLockedByThisThread() const noexcept
    {
        return holder == std::this_thread::get_id();
    }
private:
    std::thread::id holder = std::thread::id();
    std::size_t count = 0;
};

// a resource with a lock
template <typename T>
class Lockable
{
public:
    void load(const T& theResource)
    {
        const std::lock_guard<Mutex> L (lock);
        resource = theResource;
    }
//    void load(T&& theResource)
//    {
//        const std::lock_guard<Mutex> L (lock);
//        resource = std::forward<T>(theResource);
//    }
    Mutex& get(T*& theResource) noexcept
    {
        theResource = &resource;
        return lock;
    }
    Mutex& get(const T*& theResource) const noexcept
    {
        theResource = &resource;
        return lock;
    }
    Mutex& getLock() const noexcept
    {
        return lock;
    }
    T& getResource() noexcept
    {
        return resource;
    }
    const T& getResource() const noexcept
    {
        return resource;
    }
    bool isLocked() const noexcept
    {
        return lock.isLocked();
    }
    bool isLockedByThisThread() const noexcept
    {
        return lock.isLockedByThisThread();
    }
private:
    T resource;
    mutable Mutex lock;
};

// a concurrent resource that can always immediately produce a locked copy of the resource for realtime thread-safe use
template <typename T, const std::size_t numThreads>
class RealtimeConcurrent
{
public:
    void load(const T& resource)
    {
        for (auto& c : copies)
            c.load(resource);
    }
    
//    void load(T&& resource)
//    {
//        for (std::size_t i = 0; i < copies.size()-1; ++i) {
//            copies[i].load(resource);
//        }
//        copies.back().load(std::forward<T>(resource));
//    }
    
    Mutex& get(T*& resource) noexcept
    {
        // check if this thread already has a copy locked and if so return that same copy
        for (auto& c : copies) {
            if (c.isLockedByThisThread()) {
                return c.get(resource);
            }
        }
        // otherwise return a new unlocked copy
        for (auto& c : copies) {
            if (!c.isLocked()) {
                return c.get(resource);
            }
        }
        // should never get here if N+1 >= the number of different threads using this resource
        resource = nullptr;
        return dummyLock;
    }
    
    Mutex& get(const T*& resource) const noexcept
    {
        // check if this thread already has a copy locked and if so return that same copy
        for (auto& c : copies) {
            if (c.isLockedByThisThread()) {
                return c.get(resource);
            }
        }
        // otherwise return a new unlocked copy
        for (auto& c : copies) {
            if (!c.isLocked()) {
                return c.get(resource);
            }
        }
        // should never get here if N+1 >= the number of different threads using this resource
        resource = nullptr;
        return dummyLock;
    }
    
    bool tryToUpdate(const T* updatedCopy)
    {
        bool allCopiesUpdated = false;
        const std::unique_lock<std::mutex> lockedForUpdate (updateLock, std::try_to_lock);
        if (lockedForUpdate) {
            allCopiesUpdated = true;
            T* copyToUpdate = nullptr;
            for (auto& c : copies) {
                if (c.isSame(updatedCopy)) {
                    c.upToDate = true;
                } else {
                    const std::unique_lock<Mutex> copyLocked (c.get(copyToUpdate), std::try_to_lock);
                    if (copyLocked) {
                        *copyToUpdate = *updatedCopy;
                        c.upToDate = true;
                    } else {
                        c.upToDate = false;
                        allCopiesUpdated = false;
                    }
                }
            }
        }
        return allCopiesUpdated;
    }
    
    void update(const T* updatedCopy)
    {
//        // straightforward blocking update
//        const std::lock_guard<std::mutex> lockedForUpdate (updateLock);
//        T* copyToUpdate = nullptr;
//        for (auto& c : copies) {
//            if (c.isSame(updatedCopy)) {
//                c.upToDate = true;
//            } else {
//                const std::lock_guard<Mutex> copyLocked (c.get(copyToUpdate));
//                *copyToUpdate = *updatedCopy;
//                c.upToDate = true;
//            }
//        }
        // "spin" update, potentially most productive strategy if we can't get all copies updated in one thread epoch
        const std::lock_guard<std::mutex> lockedForUpdate (updateLock);
        for (auto& c : copies)
            c.upToDate = false;
        T* copyToUpdate = nullptr;
        std::size_t numUpdated = 0, i = 0;
        while (numUpdated < copies.size()) {
            if (!copies[i].upToDate) {
                if (copies[i].isSame(updatedCopy)) {
                    copies[i].upToDate = true;
                    ++numUpdated;
                } else {
                    const std::unique_lock<Mutex> copyLocked (copies[i].get(copyToUpdate), std::try_to_lock);
                    if (copyLocked) {
                        *copyToUpdate = *updatedCopy;
                        copies[i].upToDate = true;
                        ++numUpdated;
                    }
                }
            }
            i = (i+1) % copies.size();
        }
    }
    
private:
    // a lockable resource that can keep track of its updated state and check if a copy of the resource refers to this resource
    class UpdateableCopiableLockable : public Lockable<T>
    {
    public:
        bool isSame(const T* copy) const noexcept
        {
            return copy == &Lockable<T>::getResource();
        }
        bool upToDate = true;
    };
    std::array<UpdateableCopiableLockable, numThreads+1> copies; // N+1 copies for N threads b/c if one thread is in the middle of an update, up to two copies may be locked and we still want at least N-1 free copies for the other N-1 threads to be able to have immediate access to if need be
    std::mutex updateLock; // updates must be serialized because during an update we can have up to two copies locked at once.  if each thread can potentially be updating at once, we'd need more copies and how would those simultaneous updates work anyways?
    mutable Mutex dummyLock; // lock that is returned if get() should fail to produce an immediately lockable copy of the resource
};



//// client code
//ConcurrentResource<Thing, numThreads> c_resource;
//
//void realtimeFunction()
//{
//    Thing* copy;
//    std::lock_guard<Mutex> lock (c_resource.get(copy)); // won't fail to lock the copy that is returned
//    if (copy == nullptr)
//        return;
//    
//    // use the copy ...
//    
//    // if anything was changed (copy that gets returned next time around is not guaranteed to be updated, but may be...)
//    c_resource.tryToUpdate(copy);
//}
//
//void nonRealtimeFunction()
//{
//    Thing* copy;
//    std::lock_guard<Mutex> lock (c_resource.get(copy)); // won't fail to lock the copy that is returned
//    if (copy == nullptr)
//        return;
//    
//    // use the copy ...
//    
//    // if anything was changed (this will block until all copies are updated)
//    c_resource.update(copy);
//}

#endif /* ConcurrentResource_h */
