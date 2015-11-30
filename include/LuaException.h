//
//  LuaException.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include <lua.hpp>

#include <cstring>
#include <stdexcept>
#include <iostream>
#include <string>

namespace lua {
    
    namespace stack {
        
        inline void dump (lua_State *L) {
            int top = lua_gettop(L);
            for (int i = 1; i <= top; i++) {  /* repeat for each level */
                int t = lua_type(L, i);
                switch (t) {
                    case LUA_TSTRING:  /* strings */
                        std::cout << '`' << lua_tostring(L, i) << '\'';
                        break;
                        
                    case LUA_TBOOLEAN:  /* booleans */
                        std::cout << (lua_toboolean(L, i) ? "true" : "false");
                        break;
                        
                    case LUA_TNUMBER:  /* numbers */
                        std::cout << lua_tonumber(L, i);
                        break;
                        
                    default:  /* other values */
                        std::cout << lua_typename(L, t);
                        break;
                        
                }
                std::cout << "  ";  /* put a separator */
            }
            std::cout << std::endl;  /* end the listing */
        }
        
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    class LoadError : public std::runtime_error
    {
        std::string m_message;
        
    public:
        LoadError(lua_State* luaState)
            : std::runtime_error{ lua_tostring(luaState, -1) }
        {
            lua_pop(luaState, 1);
        }
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    class RuntimeError : public std::runtime_error
    {
        std::string m_message;
        
    public:
        RuntimeError(lua_State* luaState)
            : std::runtime_error{ lua_tostring(luaState, -1) }
        {
            lua_pop(luaState, 1);
        }
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    class TypeMismatchError : public std::runtime_error
    {
    public:
        TypeMismatchError(lua_State*, int index)
            : std::runtime_error{ "Type mismatch error at index " + std::to_string(index) }
        {
        }
    };
}
