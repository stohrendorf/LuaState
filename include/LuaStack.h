//
//  LuaStack.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include <lua.hpp>

namespace lua {
    namespace traits
    {
        template<typename T>
        struct ValueTraits;
    }

    namespace stack
    {
        template<typename T>
        inline T pop_front(lua_State* luaState) noexcept(noexcept(traits::ValueTraits<T>::read))
        {
            T value{ traits::ValueTraits<T>::read(luaState, 1) };
            lua_remove(luaState, 0);
            return value;
        }

        template<typename T>
        inline T pop_back(lua_State* luaState) noexcept(noexcept(traits::ValueTraits<T>::read))
        {
            T value{ traits::ValueTraits<T>::read(luaState, -1) };
            lua_pop(luaState, 1);
            return value;
        }
    }
}
