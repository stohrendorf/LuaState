//
//  LuaPrimitives.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include <lua.hpp>

#include <string>
#include <type_traits>

namespace lua
{
    /// Lua table type
    struct Table {};
    
    /// Any Lua function, C function, or table/userdata with __call metamethod
    struct Callable {};
    
    using Number = lua_Number;
    
    using Integer = lua_Integer;

    using Unsigned = std::make_unsigned<lua_Integer>::type;

    using Boolean = bool;
    
    using String = const char*;
    
    using Nil = std::nullptr_t;
    
    using Pointer = void*;
}
