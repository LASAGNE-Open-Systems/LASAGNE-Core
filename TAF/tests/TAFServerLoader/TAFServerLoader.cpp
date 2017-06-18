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
#define TAFSERVERLOADER_CPP

#if defined(_WIN32)

#include <Windows.h>

typedef HMODULE     SHLIB_MODULE;

# define LOAD_LIBRARY(libName)              (::LoadLibrary(libName))
# define FREE_LIBRARY(libModule)            (::FreeLibrary(libModule) ? 0 : -1)
# define PROC_ADDRESS(libModule, fnName)    (reinterpret_cast<void *>(::GetProcAddress(libModule,fnName)))
# define OS_SLEEP(msec)                     (::Sleep(msec))

# define LOADER_DLL_SUFFIX          ".dll"

#else // Linux

#include <dlfcn.h>
#include <unistd.h>

typedef void *      SHLIB_MODULE;

# define LOAD_LIBRARY(libName)              (::dlopen(libName, RTLD_LAZY))
# define FREE_LIBRARY(libModule)            (::dlclose(libModule))
# define PROC_ADDRESS(libModule, fnName)    (reinterpret_cast<void *>(::dlsym(libModule,fnName)))
# define OS_SLEEP(msec)                     (::sleep(msec / 1000))

# define LOADER_DLL_SUFFIX          ".so"

#endif

#include <cstdio>
#include <cstring>
#include <algorithm>

#include <iostream>
#include <string>
#include <list>

#define TAF_DLL_NAME            "TAF"

#define TAF_INIT_FUNCTION_NAME  "_init_TAFServer"
#define TAF_WAIT_FUNCTION_NAME  "_wait_TAFServer"
#define TAF_FINI_FUNCTION_NAME  "_fini_TAFServer"

extern "C" {
    typedef int(*TAFSERVER_INIT_FUNCTION)(int argc, char *argv[]);
    typedef int(*TAFSERVER_WAIT_FUNCTION)(unsigned);
    typedef int(*TAFSERVER_FINI_FUNCTION)(void);
}

namespace {

    template <typename T>
    const T& _maximum(const T &t1, const T &t2) { return t1 > t2 ? t1 : t2; }

    template <typename T>
    const T& _minimum(const T &t1, const T &t2) { return t1 > t2 ? t2 : t1; }

    class DLLManager : protected std::list<std::string>
    {
    public:

        DLLManager(const std::string &dll_name);
        virtual ~DLLManager(void)
        {
            if (this->libModule_) {
                FREE_LIBRARY(this->libModule_); this->libModule_ = 0;
            }
        }

    protected:

        SHLIB_MODULE    libModule_;
    };

    DLLManager::DLLManager(const std::string &dll_name) : libModule_(0)
    {
        static const char *dll_name_formats[] = {
#if defined(_WIN32)
# if defined(_DEBUG)
            "%sd%s",    // TAFd.dll
            "%s%s",     // TAF.dll
# else
            "%s%s",     // TAF.dll
            "%sd%s",    // TAFd.dll
# endif
#else  // Linux
            "lib%s%s",  // libTAF.so
            "%s%s",     // TAF.so
#endif
            0
        };

        // Build the array of DLL names to try on this platform by applying the
        // proper prefixes and/or suffixes to the specified dll_name.

        // 1. Separate the dll_name into the dir part and the file part. We
        // only decorate the file part to determine the names to try loading.
        std::string base_dir, base_file(dll_name), base_suffix(LOADER_DLL_SUFFIX);

        { // Always try both seperation character versions.
            int pos = _maximum(int(dll_name.find_last_of('/')), int(dll_name.find_last_of('\\')));
            if (pos++ > 0) {
                base_dir.assign(dll_name.substr(0, pos)); base_file.assign(dll_name.substr(pos));
            }
        }

        // 2. Locate the file suffix, if there is one. Move the '.' and the
        // suffix to base_suffix.

        {
            const std::string temp_file(base_file);
            int pos = int(temp_file.find_last_of('.'));
            if (pos > 0) {
                base_suffix.assign(temp_file.substr(pos)); base_file.assign(temp_file.substr(0, pos));
            }
        }

        /*
            1) If on Windows we dont do the prefix entries (linux prefix == "lib")
            2) If on Windows and currently building debug we order the dll entries
               to put the decorated name entry first (i.e. TAFd.dll)
            3) On Linux we put the un-decorated prefixed name first (decoration is unlikely on Linux)
            4) Last Entry is always the original name as passed...
        */

        for (size_t i = 0;;i++) {

            const char * dll_name_format = dll_name_formats[i];

            if (dll_name_format == 0) {
                break;  // We Are at the end
            }

            char s[128]; std::string try_name(base_dir);

            if (::sprintf(s, dll_name_format, base_file.c_str(), base_suffix.c_str()) > 0) {
                try_name.append(s); bool add_entry = true; // Say we will add the entry
                for (const_iterator it = this->begin(); it != this->end(); it++) {
                    if (*it == try_name) {  // Have we got this already?
                        add_entry = false; break;
                    }
                }

                if (add_entry) {  // Add Entry if not already there
                    this->push_back(try_name);
                }
            }
        }

        this->push_back(dll_name);
    }

    class TAFServerLoader : DLLManager
    {
    public:

        TAFServerLoader(void);
        ~TAFServerLoader(void);

        int init(int argc, char *argv[]);
        int wait_completion(unsigned = 0); // Wait 0 == infinately
        int fini(void);

    private:

        std::string libName_;
        bool        libInit_;
    };

    TAFServerLoader::TAFServerLoader(void) : DLLManager(TAF_DLL_NAME)
        , libInit_(false)
    {
        for (const_iterator it = this->begin(); it != this->end(); it++) {
            if ((this->libModule_ = LOAD_LIBRARY(it->c_str())) != 0) {
                this->libName_ = *it; return;
            }
        }

        throw "Unable-to-load-DLL";
    }

    TAFServerLoader::~TAFServerLoader(void)
    {
        if (this->libInit_) this->fini();
    }

    int
    TAFServerLoader::init(int argc, char *argv[])
    {
        void *fn = PROC_ADDRESS(this->libModule_, TAF_INIT_FUNCTION_NAME);

        if (fn) {
            TAFSERVER_INIT_FUNCTION taf_server_init = reinterpret_cast<TAFSERVER_INIT_FUNCTION>(fn);
            if (taf_server_init) {
                if (taf_server_init(argc, argv) ? false : this->libInit_ = true) {
                    return 0;
                }
            }
        }

        throw "Unable-to-locate-" TAF_INIT_FUNCTION_NAME "-function-in-DLL";
    }

    int
    TAFServerLoader::wait_completion(unsigned secs)
    {
        void *fn = PROC_ADDRESS(this->libModule_, TAF_WAIT_FUNCTION_NAME);

        if (fn) {
            TAFSERVER_WAIT_FUNCTION taf_server_wait = reinterpret_cast<TAFSERVER_WAIT_FUNCTION>(fn);
            if (taf_server_wait) {
                for (int wait_rtn = taf_server_wait(secs); wait_rtn;) {
                    return wait_rtn;
                }
                this->libInit_ = false; return 0;
            }
        }

        throw "Unable-to-locate-" TAF_WAIT_FUNCTION_NAME "-function-in-DLL";
    }

    int
    TAFServerLoader::fini(void)
    {
        void *fn = PROC_ADDRESS(this->libModule_, TAF_FINI_FUNCTION_NAME);

        if (fn) {
            TAFSERVER_FINI_FUNCTION taf_server_fini = reinterpret_cast<TAFSERVER_FINI_FUNCTION>(fn);
            if (taf_server_fini) {
                this->libInit_ = false; return taf_server_fini();
            }
        }

        throw "Unable-to-locate-" TAF_FINI_FUNCTION_NAME "-function-in-DLL";
    }
}
