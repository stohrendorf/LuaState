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

        //////////////////////////////////////////////////////////////////////////////////////////////

        inline int top(lua_State* luaState) {
            return lua_gettop(luaState);
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        inline void settop(lua_State* luaState, int n) {
            lua_settop(luaState, n);
        }

        inline void pop(lua_State* luaState, int n) {
            lua_pop(luaState, n);
        }

        template<typename T>
        inline T pop_front(lua_State* luaState) {
            T value = traits::ValueTraits<T>::read(luaState, 1);
            lua_remove(luaState, 0);
            return value;
        }

        template<typename T>
        inline T pop_back(lua_State* luaState) {
            T value = traits::ValueTraits<T>::read(luaState, -1);
            pop(luaState, 1);
            return value;
        }

        //////////////////////////////////////////////////////////////////////////////////////////////

        inline void get_global(lua_State* luaState, const char* name) {
            lua_getglobal(luaState, name);
        }

    }
}
