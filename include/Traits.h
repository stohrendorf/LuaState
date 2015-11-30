//
//  Traits.h
//  LuaState
//
//  Created by Simon Mikuda on 22/03/14.
//
//  See LICENSE and README.md files//

#pragma once

#include "LuaPrimitives.h"

#include <cmath>
#include <functional>
#include <limits>

namespace lua { namespace traits {
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    template<size_t...>
    struct index_tuple{};
    
    template<size_t I, typename IndexTuple, typename... Types>
    struct make_indexes_impl;
    
    template<size_t I, size_t... Indexes, typename T, typename ... Types>
    struct make_indexes_impl<I, index_tuple<Indexes...>, T, Types...>
    {
        using type = typename make_indexes_impl<I + 1, index_tuple<Indexes..., I>, Types...>::type;
    };
    
    template<size_t I, size_t... Indexes>
    struct make_indexes_impl<I, index_tuple<Indexes...> >
    {
        typedef index_tuple<Indexes...> type;
    };
    
    template<typename... Types>
    struct make_indexes : make_indexes_impl<0, index_tuple<>, Types...>
    {};
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    template<typename Ret, typename... Args, size_t... Indexes >
    Ret callHelper(std::function<Ret(Args...)> func, index_tuple<Indexes...>, std::tuple<Args...>&& tup)
    {
        return func( std::forward<Args>(std::get<Indexes>(tup))... );
    }
    
    template<typename Ret, typename... Args>
    Ret call(std::function<Ret(Args...)> pf, std::tuple<Args...>&& tup)
    {
        return callHelper(pf, typename make_indexes<Args...>::type(), std::forward<std::tuple<Args...>>(tup));
    }
    
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    template <std::size_t... Is>
    struct indexes {};
    
    template <std::size_t N, std::size_t... Is>
    struct indexes_builder : indexes_builder<N-1, N-1, Is...> {};
    
    template <std::size_t... Is>
    struct indexes_builder<0, Is...> {
        typedef indexes<Is...> index;
    };

    template<typename T>
    using RemoveCVR = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    template<typename T>
    struct ValueTraits;

    template<typename T>
    struct ValueTraits<T&> : ValueTraits<RemoveCVR<T>>
    {};

    template<typename T>
    struct IntValueTraits
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        static_assert(std::is_signed<T>::value, "T must be signed");
        static_assert(!std::is_floating_point<T>::value, "T must not be floating point");

        static inline T read(lua_State* luaState, int index) {
            return static_cast<T>( lua_tointeger(luaState, index) );
        }

        static inline bool check(lua_State* luaState, int index)
        {
            if (!lua_isnumber(luaState, index))
                return false;

            const auto eps = std::numeric_limits<T>::epsilon();
            Number number = lua_tonumber(luaState, index);
            return std::abs(number - static_cast<T>(number + eps)) <= eps;
        }

        static inline void get(lua_State* luaState, int index, T key) {
            lua_rawgeti(luaState, index, key);
        }

        static inline int push(lua_State* luaState, T value) {
            lua_pushnumber(luaState, value);
            return 1;
        }
    };

    template<>
    struct ValueTraits<long long> : public IntValueTraits<long long>
    {
    };

    template<>
    struct ValueTraits<long> : public IntValueTraits<long>
    {
    };

    template<>
    struct ValueTraits<int> : public IntValueTraits<int>
    {
    };

    template<>
    struct ValueTraits<short> : public IntValueTraits<short>
    {
    };

    template<>
    struct ValueTraits<signed char> : public IntValueTraits<signed char>
    {
    };

    template<>
    struct ValueTraits<lua::Boolean>
    {
        static inline lua::Boolean read(lua_State* luaState, int index) {
            return lua_toboolean(luaState, index) != 0;
        }

        static inline bool check(lua_State* luaState, int index)
        {
            return lua_isboolean(luaState, index) != 0;
        }

        static inline void get(lua_State* luaState, int index, lua::Boolean key) {
            lua_rawgeti(luaState, index, key);
        }

        static inline int push(lua_State* luaState, lua::Boolean value) {
            lua_pushboolean(luaState, value);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::Number>
    {
        static inline lua::Number read(lua_State* luaState, int index) {
            return lua_tonumber(luaState, index);
        }

        static inline bool check(lua_State* luaState, int index)
        {
            return lua_isnumber(luaState, index) != 0;
        }

        static inline int push(lua_State* luaState, lua::Number value) {
            lua_pushnumber(luaState, value);
            return 1;
        }
    };

    template<>
    struct ValueTraits<float>
    {
        static inline float read(lua_State* luaState, int index) {
            return static_cast<float>(lua_tonumber(luaState, index));
        }

        static inline bool check(lua_State* luaState, int index)
        {
            return lua_isnumber(luaState, index) != 0;
        }

        static inline int push(lua_State* luaState, float value) {
            lua_pushnumber(luaState, value);
            return 1;
        }
    };

    template<>
    struct ValueTraits<long double>
    {
        static inline long double read(lua_State* luaState, int index) {
            return static_cast<long double>(lua_tonumber(luaState, index));
        }

        static inline bool check(lua_State* luaState, int index)
        {
            return lua_isnumber(luaState, index) != 0;
        }

        static inline int push(lua_State* luaState, long double value) {
            lua_pushnumber(luaState, value);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::String>
    {
        static inline lua::String read(lua_State* luaState, int index) {
            return lua_tostring(luaState, index);
        }

        static inline bool check(lua_State* luaState, int index)
        {
            // Lua is treating numbers also like strings, because they are always convertible to string
            if (lua_isnumber(luaState, index))
                return false;

            return lua_isstring(luaState, index) != 0;
        }

        static inline void get(lua_State* luaState, int index, lua::String key) {
            lua_getfield(luaState, index, key);
        }

        static inline int push(lua_State* luaState, lua::String value) {
            lua_pushstring(luaState, value);
            return 1;
        }

        static inline int push(lua_State* luaState, lua::String value, std::size_t length) {
            lua_pushlstring(luaState, value, length);
            return 1;
        }
    };

    template<>
    struct ValueTraits<std::string>
    {
        static inline std::string read(lua_State* luaState, int index) {
            return lua_tostring(luaState, index);
        }

        static inline bool check(lua_State* luaState, int index)
        {
            // Lua is treating numbers also like strings, because they are always convertible to string
            if (lua_isnumber(luaState, index))
                return false;

            return lua_isstring(luaState, index) != 0;
        }

        static inline void get(lua_State* luaState, int index, std::string key) {
            lua_getfield(luaState, index, key.c_str());
        }

        static inline int push(lua_State* luaState, std::string value) {
            lua_pushstring(luaState, value.c_str());
            return 1;
        }

        static inline int push(lua_State* luaState, std::string value, std::size_t length) {
            lua_pushlstring(luaState, value.substr(0, length).c_str(), std::min(length, value.length()));
            return 1;
        }
    };

    template<std::size_t N>
    struct ValueTraits<const char[N]> : ValueTraits<lua::String>
    {
    };

    template<std::size_t N>
    struct ValueTraits<char[N]> : ValueTraits<lua::String>
    {
    };

    template<>
    struct ValueTraits<lua::Nil>
    {
        static inline lua::Nil read(lua_State*, int) {
            return nullptr;
        }

        static inline bool check(lua_State* luaState, int index)
        {
            return lua_isnoneornil(luaState, index) != 0;
        }

        static inline int push(lua_State* luaState, lua::Nil) {
            lua_pushnil(luaState);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::Pointer>
    {
        static inline lua::Pointer read(lua_State* luaState, int index) {
            return lua_touserdata(luaState, index);
        }

        static inline bool check(lua_State* luaState, int index)
        {
            return lua_islightuserdata(luaState, index) != 0;
        }

        static inline int push(lua_State* luaState, lua::Pointer value) {
            lua_pushlightuserdata(luaState, value);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::Table>
    {
        static inline lua::Table read(lua_State*, int) {
            return {};
        }

        static inline bool check(lua_State* luaState, int index)
        {
            return lua_istable(luaState, index) != 0;
        }

        static inline void get(lua_State* luaState, int index, lua::Table) {
            lua_gettable(luaState, index);
        }

        static inline int push(lua_State* luaState, lua::Table) {
            lua_newtable(luaState);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::Callable>
    {
        static inline bool check(lua_State* luaState, int index)
        {
            bool isCallable = lua_isfunction(luaState, index) != 0 || lua_iscfunction(luaState, index) != 0;

            if (!isCallable) {
                lua_getmetatable(luaState, index);
                if (lua_istable(luaState, -1)) {
                    lua_pushstring(luaState, "__call");
                    lua_rawget(luaState, -2);
                    isCallable = !lua_isnil(luaState, -1);
                    lua_pop(luaState, 1);
                }
                lua_pop(luaState, 1);
            }

            return isCallable;
        }
    };

    template<typename... Args>
    struct ValueTraits<std::tuple<Args...>>
    {
    private:
        template<std::size_t... Indexes>
        static void pushTuple(lua_State* luaState, traits::index_tuple<Indexes...>, std::tuple<Args&&...>&& tup)
        {
            pushRec(luaState, std::get<Indexes>(tup)...);
        }

        template<typename T1, typename... Ts>
        static inline int pushRec(lua_State* luaState, T1&& value1,  Ts&&... values) {
            ValueTraits<T1>::push(luaState, std::forward<T1>(value1));
            pushRec(luaState, std::forward<Ts>(values)...);
            return static_cast<int>(sizeof...(Ts) + 1);
        }

        static inline int pushRec(lua_State*) {
            return 0;
        }

    public:
        static inline int push(lua_State* luaState, std::tuple<Args&&...>&& value) {
            pushTuple(luaState, typename traits::make_indexes<Args...>::type(), std::forward<std::tuple<Args&&...>>(value));
            return static_cast<int>(sizeof...(Args));
        }

        static inline int push(lua_State* luaState, Args&&... args)
        {
            return pushRec(luaState, std::forward<Args>(args)...);
        }
    };
}}
