//
//  LuaReturn.h
//  LuaState
//
//  Created by Simon Mikuda on 18/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include "LuaValue.h"
#include "Traits.h"

#include <tuple>

namespace lua {
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    namespace stack {
        
        
        //////////////////////////////////////////////////////////////////////////////////////////////
        /// Function get single value from lua stack
        template<typename T>
        inline T readValue(::lua_State* luaState,
                           detail::DeallocQueue* deallocQueue,
                           int stackTop)
        {
            static_assert(std::is_same<traits::RemoveCVR<T>, T>::value, "T must not be CV-qualified or a reference");
            return lua::Value(std::make_shared<detail::StackItem>(luaState, deallocQueue, stackTop - 1, 1, 0)).to<T>();
        }

        /// Function creates indexes for mutli values and get them from stack
        template<typename... Ts, std::size_t... Is>
        inline std::tuple<Ts...> unpackMultiValues(::lua_State* luaState,
                                                                      detail::DeallocQueue* deallocQueue,
                                                                      int stackTop,
                                                                      traits::Indices<Is...>)
        {
            return std::make_tuple(readValue<Ts>(luaState, deallocQueue, Is + stackTop)...);
        }

        /// Function expects that number of elements in tuple and number of pushed values in stack are same. Applications takes care of this requirement by popping overlapping values before calling this function
        template<typename... Ts>
        inline std::tuple<Ts...> get_and_pop(::lua_State* luaState,
                                                                detail::DeallocQueue* deallocQueue,
                                                                int stackTop)
        {
            return unpackMultiValues<Ts...>(luaState, deallocQueue, stackTop, typename traits::MakeIndices<sizeof...(Ts)>::Type());
        }

        template<>
        inline std::tuple<> get_and_pop<>(::lua_State*,
                                          detail::DeallocQueue*,
                                          int)
        {
            return {};
        }
    }
    

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Class for automaticly cas lua::Function instance to multiple return values with lua::tie
    template <typename ... Ts>
    class Return final
    {
        /// Return values
        std::tuple<Ts&...> m_tiedValues;
        
    public:
        
        /// Constructs class with given arguments
        ///
        /// @param args    Return values
        explicit Return(Ts&... args)
            : m_tiedValues(args...)
        {
        }
        
        /// Operator sets values to std::tuple
        ///
        /// @param function     Function being called
        void operator=(const Value& value)
        {
            
            int requiredValues = std::min<int>(sizeof...(Ts), value.m_stack->pushed);
            
            // When there are more returned values than variables in tuple, we will clear values that are not needed
            if (requiredValues < value.m_stack->grouped + 1)
            {
                
                int currentStackTop = lua_gettop(value.m_stack->state);
                
                // We will check if we haven't pushed some other new lua::Value to stack
                if (value.m_stack->top + value.m_stack->pushed == currentStackTop)
                    lua_settop(value.m_stack->state, value.m_stack->top + requiredValues);
                else
                    value.m_stack->deallocQueue->push(detail::DeallocStackItem(value.m_stack->top, value.m_stack->pushed));
            }
            
            // We will take pushed values and distribute them to returned lua::Values
            value.m_stack->pushed = 0;
            
            m_tiedValues = stack::get_and_pop<traits::RemoveCVR<Ts>...>(value.m_stack->state, value.m_stack->deallocQueue, value.m_stack->top + 1);
        }
        
    };
    
    /// Use this function when you want to retrieve multiple return values from lua::Function
    template <typename ... Ts>
    Return<Ts&...> tie(Ts&... args)
    {
        return Return<Ts&...>(args...);
    }
}
