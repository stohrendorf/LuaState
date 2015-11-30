//
//  LuaFunctor.h
//  LuaState
//
//  Created by Simon Mikuda on 22/03/14.
//
//  See LICENSE and README.md files
//

#pragma once

#include "LuaState.h"

#include <type_traits>

namespace lua {
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Base functor class with call function. It is used for registering lamdas, or regular functions
    struct BaseFunctor
    {
        BaseFunctor() = default;
        
        virtual ~BaseFunctor() noexcept = default;
        
        /// In Lua numbers of argumens can be different so here we will handle these situations.
        ///
        /// @param luaState     Pointer of Lua state
        inline void prepareFunctionCall(lua_State* luaState, int requiredValues) {
            
            // First item is our pushed userdata
            if (stack::top(luaState) > requiredValues + 1) {
                stack::settop(luaState, requiredValues + 1);
            }
        }
        
        /// Virtual function that will make Lua call to our functor.
        ///
        /// @param luaState     Pointer of Lua state
        virtual int call(lua_State* luaState) = 0;
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Functor with return values
    template<typename Ret, typename ... Args>
    struct Functor : public BaseFunctor {
        std::function<Ret(Args...)> function;
        
        /// Constructor creates functor to be pushed to Lua interpret
        Functor(std::function<Ret(Args...)> function)
            : BaseFunctor()
            , function(function)
        {
        }
        
        /// We will make Lua call to our functor.
        ///
        /// @note When we call function from Lua to C, they have their own stack, where in the first position is our binded userdata and next position are pushed arguments
        ///
        /// @param luaState     Pointer of Lua state
        int call(lua_State* luaState) override {
            Ret value = traits::call(function, stack::get_and_pop<Args...>(luaState, nullptr, 2));
            return traits::ValueTraits<Ret>::push(luaState, std::forward<Ret>(value));
        }
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Functor with no return values
    template <typename ... Args>
    struct Functor<void, Args...> : public BaseFunctor {
        std::function<void(Args...)> function;
        
        /// Constructor creates functor to be pushed to Lua interpret
        Functor(std::function<void(Args...)> function)
            : BaseFunctor()
            , function(function)
        {
        }
        
        /// We will make Lua call to our functor.
        ///
        /// @note When we call function from Lua to C, they have their own stack, where in the first position is our binded userdata and next position are pushed arguments
        ///
        /// @param luaState     Pointer of Lua state
        int call(lua_State* luaState) override {
            traits::call(function, stack::get_and_pop<Args...>(luaState, nullptr, 2));
            return 0;
        }
    };

    namespace traits
    {
        template<typename Ret, typename... Args>
        struct ValueTraits<std::function<Ret(Args...)>>
        {
            static inline int push(lua_State* luaState, std::function<Ret(Args...)> function) {
                BaseFunctor** udata = (BaseFunctor **)lua_newuserdata(luaState, sizeof(BaseFunctor *));
                *udata = new Functor<Ret, Args...>(function);

                luaL_getmetatable(luaState, "luaL_Functor");
                lua_setmetatable(luaState, -2);
                return 1;
            }
        };

        template<typename Ret, typename... Args>
        struct ValueTraits<Ret(Args...)> : ValueTraits<std::function<Ret(Args...)>>
        {
        };

        template<typename Ret, typename... Args>
        struct ValueTraits<Ret(*)(Args...)> : ValueTraits<std::function<Ret(Args...)>>
        {
        };

        template <typename T>
        struct ValueTraits : public ValueTraits<decltype(&T::operator())>
        {
        };

        template <typename C, typename R, typename... Args>
        struct ValueTraits<R(C::*)(Args...)> : ValueTraits<R(Args...)>
        {
        };

        template <typename C, typename R, typename... Args>
        struct ValueTraits<R(C::*)(Args...) const> : ValueTraits<R(Args...)>
        {
        };
    }
}
