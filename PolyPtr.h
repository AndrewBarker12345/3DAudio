/*
 PolyPtr.h:  a polymorphic smart pointer that deep copies
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

#ifndef PolyPtr_h
#define PolyPtr_h

#include <cstddef>
#include <utility>

template <class T>
class PolyPtr
{
public:
    // default ctor sets up a nullptr
    constexpr PolyPtr(std::nullptr_t n = nullptr) noexcept : ptr(nullptr) {}
    
    // destructor frees the memory so you can rely on RAII to do the cleanup
    ~PolyPtr() { delete ptr; }
    
    // ctor for creating a PolyPtr that assumes the ownership of some other raw pointer
    explicit PolyPtr(T* p) noexcept : ptr(std::move(p)) {}
    
    // copy ctor that implements the polymporhic deep copying
    PolyPtr(const PolyPtr& p) : ptr(p ? p.ptr->clone() : nullptr) {}
    
    // copy ctor that allows you to create a parent PolyPtr from a child PolyPtr
    template <class Other>
    PolyPtr(const PolyPtr<Other>& p) : ptr(p ? new Other(*p) : nullptr) {}
    
    // move ctor that swaps the underlying pointers
    PolyPtr(PolyPtr&& p) noexcept : PolyPtr() { swap(p); }
    
    // move ctor that allows parent / child pointer swaps
    template <class Other>
    PolyPtr(PolyPtr<Other>&& p) noexcept : PolyPtr() { swap(p); }
    
    // copy assignment that is implemented with the copy and swap idiom
    PolyPtr& operator=(PolyPtr p) noexcept
    {
        swap(p);
        return *this;
    }
    
    // copy assigment for parentPtr = childPtr type assignments
    template <class Other>
    PolyPtr& operator=(PolyPtr<Other> p) noexcept
    {
        swap(p);
        return *this;
    }
    
    // raw pointer assignment
    PolyPtr& operator=(T* p) noexcept
    {
        reset(p);
        return *this;
    }
    
    // swap this PolyPtr with another
    void swap(PolyPtr& p) noexcept
    {
        using std::swap;
        swap(ptr, p.ptr);
        /* "using std::swap` then calling swap (without std::) allows for Koenig look up of a type specific swap first. If that does not exist then the std::swap version will be used by the compiler. */
    }
    
    // allows internal access to PolyPtr<Other>::ptr
    template <class Other> friend class PolyPtr;
    
    // swap that allows parent/child PolyPtr swapping
    template <class Other>
    void swap(PolyPtr<Other>& p) noexcept
    {
        const auto tmp = ptr;
        ptr = std::move(dynamic_cast<T*>(p.get()));  // shouldn't produce exceptions because we're casting a pointer type, not a reference
        p.ptr = std::move(dynamic_cast<Other*>(tmp));
    }
    
    // take over ownership of a raw pointer, or just delete the currently owned one if it exists
    void reset(T* newPtr = nullptr) noexcept
    {
        if (ptr)
            delete ptr;
        ptr = newPtr;
    }
    
    // const / non-const access to underlying raw pointer
    T* get() noexcept { return ptr; }
    const T* get() const noexcept { return ptr; }
    
    // pointer access operators
    T* operator->() noexcept { return ptr; }
    const T* operator->() const noexcept { return ptr; }
    
    // dereference operators
    T& operator*() noexcept { return *ptr; }
    const T& operator*() const noexcept { return *ptr; }
    
    // allows a quick "if (somePolyPtr)" to check for nullptr
    explicit operator bool() const noexcept { return ptr; }
    
    // implicit conversion operator so you don't have to use get() to convert a PolyPtr to T*
    operator T*() const noexcept { return ptr; }
    
private:
    // the underlying raw pointer is encapsulated and protected from unecessary harm
    T* ptr;
};

// a swap free function to swap two different types (parent/child) of PolyPtrs
template <class T1, class T2>
void swap(PolyPtr<T1>& p1, PolyPtr<T2>& p2) noexcept {
    p1.swap(p2);
}

// the recommended, easy, std::make_unique-like way of creating a PolyPtr
template <class T, class... Args>
PolyPtr<T> makePolyPtr(Args&&... args) {
    return PolyPtr<T>(new T(std::forward<Args>(args)...));
}

#endif /* PolyPtr_h */