//
//  LuaFunctor.h
//  LuaState
//
//  Created by Simon Mikuda on 22/03/14.
//
//  See LICENSE and README.md files
//

#pragma once

#include "LuaReturn.h"

#include <functional>
#include <type_traits>

namespace lua {
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Base functor class with call function. It is used for registering lamdas, or regular functions
    struct BaseFunctor
    {
        template<typename Ret, typename... Args, size_t... Indexes >
        static Ret callHelper(std::function<Ret(Args...)> func, traits::IndexTuple<Indexes...>, std::tuple<Args...>&& args)
        {
            return func( std::forward<Args>(std::get<Indexes>(args))... );
        }

        template<typename Ret, typename... Args>
        static Ret call(std::function<Ret(Args...)> pf, std::tuple<Args...>&& tup)
        {
            return callHelper(pf, typename traits::MakeIndexTuple<Args...>::Type(), std::forward<std::tuple<Args...>>(tup));
        }


        explicit BaseFunctor() = default;
        
        virtual ~BaseFunctor() noexcept = default;
        
        /// In Lua numbers of argumens can be different so here we will handle these situations.
        ///
        /// @param luaState     Pointer of Lua state
        inline void prepareFunctionCall(lua_State* luaState, int requiredValues) {
            
            // First item is our pushed userdata
            if (lua_gettop(luaState) > requiredValues + 1) {
                lua_settop(luaState, requiredValues + 1);
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
    struct Functor : public BaseFunctor
    {
        std::function<Ret(Args...)> function;
        
        /// Constructor creates functor to be pushed to Lua interpret
        explicit Functor(std::function<Ret(Args...)> function)
            : BaseFunctor()
            , function(function)
        {
        }
        
        /// We will make Lua call to our functor.
        ///
        /// @note When we call function from Lua to C, they have their own stack, where in the first position is our binded userdata and next position are pushed arguments
        ///
        /// @param luaState     Pointer of Lua state
        int call(lua_State* luaState) override
        {
            Ret value = BaseFunctor::call(function, stack::get_and_pop<Args...>(luaState, nullptr, 2));
            return traits::ValueTraits<Ret>::push(luaState, std::forward<Ret>(value));
        }
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Functor with no return values
    template <typename ... Args>
    struct Functor<void, Args...> : public BaseFunctor
    {
        std::function<void(Args...)> function;
        
        /// Constructor creates functor to be pushed to Lua interpret
        explicit Functor(std::function<void(Args...)> function)
            : BaseFunctor()
            , function(function)
        {
        }
        
        /// We will make Lua call to our functor.
        ///
        /// @note When we call function from Lua to C, they have their own stack, where in the first position is our binded userdata and next position are pushed arguments
        ///
        /// @param luaState     Pointer of Lua state
        int call(lua_State* luaState) override
        {
            BaseFunctor::call(function, stack::get_and_pop<Args...>(luaState, nullptr, 2));
            return 0;
        }
    };

    namespace traits
    {
        template<typename Ret, typename... Args>
        struct ValueTraits<std::function<Ret(Args...)>>
        {
            static inline int push(lua_State* luaState, std::function<Ret(Args...)> function) {
                BaseFunctor** udata{ static_cast<BaseFunctor**>( lua_newuserdata(luaState, sizeof(BaseFunctor *)) ) };
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
