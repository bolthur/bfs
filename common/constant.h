// Copyright (C) 2022 - 2023 bolthur project.
//
// This file is part of bolthur/bfs.
//
// bolthur/bfs is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// bolthur/bfs is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with bolthur/bfs.  If not, see <http://www.gnu.org/licenses/>.

/**
 * @file common/constant.h
 */

#ifndef _COMMON_CONSTANT_H
#define _COMMON_CONSTANT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
  #define INITIALIZER(f) \
    static void f(void); \
    struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
    static void f(void)
#elif defined(_MSC_VER)
  #pragma section(".CRT$XCU",read)
  #define INITIALIZER2_(f,p) \
    static void f(void); \
    __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
    __pragma(comment(linker,"/include:" p #f "_")) \
    static void f(void)
  #ifdef _WIN64
    #define INITIALIZER(f) INITIALIZER2_(f,"")
  #else
    #define INITIALIZER(f) INITIALIZER2_(f,"_")
  #endif
#else
  #define INITIALIZER(f) \
    static void f(void) __attribute__((constructor)); \
    static void f(void)
#endif

#ifndef __unused
  #if defined(_MSC_VER)
    #define __unused
  #else
    #define __unused __attribute__((unused))
  #endif
#endif

#ifndef __inline
  #define __inline inline
#endif

#ifdef __cplusplus
}
#endif

#endif
