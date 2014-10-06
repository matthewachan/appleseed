
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2014 Esteban Tovagliari, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Interface header.
#include "sharedlibrary.h"

// Standard headers.
#include <string>
#ifdef _WIN32
    #include "foundation/platform/windows.h"
// Unix
#else
    #include <dlfcn.h>
#endif

using namespace std;

namespace foundation
{


//
// ExceptionCannotLoadSharedLib class implementation.
//

ExceptionCannotLoadSharedLib::ExceptionCannotLoadSharedLib(
    const char* path,
    const char* error_msg)
  : Exception()
{
    string err("Cannot load shared library ");
    err += path;
    err += ". Error = ";
    err += error_msg;
    set_what(err.c_str());
}


//
// ExceptionCannotLoadSharedLib class implementation.
//

ExceptionSharedLibCannotGetSymbol::ExceptionSharedLibCannotGetSymbol(
    const char* symbol_name,
    const char* error_msg)
  : Exception()
{
    string err("Cannot get symbol ");
    err += symbol_name;
    err += ". Error = ";
    err += error_msg;
    set_what(err.c_str());
}


// ------------------------------------------------------------------------------------------------
// Windows.
// ------------------------------------------------------------------------------------------------

#ifdef _WIN32

struct SharedLibrary::Impl
{
    explicit Impl(const char* path)
    {
        m_handle = LoadLibraryA(path);

        if (!m_handle)
        {
            throw ExceptionCannotLoadSharedLib(
                path,
                GetLastError());
        }
    }

    ~Impl()
    {
        FreeLibrary(handle);
    }

    void* get_symbol(const char* name, bool no_throw) const
    {
        void* symbol = GetProcAddress(m_handle, name);

        if (!symbol && !no_throw)
        {
            throw ExceptionSharedLibCannotGetSymbol(
                name,
                GetLastError());
        }

        return symbol;
    }

    HMODULE m_handle;
};


// ------------------------------------------------------------------------------------------------
// Unix.
// ------------------------------------------------------------------------------------------------

#else

struct SharedLibrary::Impl
{
    explicit Impl(const char* path)
    {
        m_handle = dlopen(path, RTLD_NOW | RTLD_GLOBAL);

        if (!m_handle)
        {
            throw ExceptionCannotLoadSharedLib(
                path,
                dlerror());
        }
    }

    ~Impl()
    {
        dlclose(m_handle);
    }

    void* get_symbol(const char* name, bool no_throw) const
    {
        void* symbol = dlsym(m_handle, name);

        if (!symbol && !no_throw)
        {
            throw ExceptionSharedLibCannotGetSymbol(
                name,
                dlerror());
        }

        return symbol;
    }

    void* m_handle;
};

#endif

//
// SharedLibrary class implementation.
//

SharedLibrary::SharedLibrary(const char* path)
  : impl(new Impl(path))
{
}

SharedLibrary::~SharedLibrary()
{
    delete impl;
}

void SharedLibrary::release()
{
    delete this;
}

void* SharedLibrary::get_symbol(const char* name, bool no_throw) const
{
    return impl->get_symbol(name, no_throw);
}

}   // namespace foundation
