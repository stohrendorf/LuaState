//
//  Traits.h
//  LuaState
//
//  Created by Simon Mikuda on 22/03/14.
//
//  See LICENSE and README.md files//

#pragma once

#include "LuaPrimitives.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

namespace lua { namespace traits {
    
    //////////////////////////////////////////////////////////////////////////////////////////////

    template<size_t...>
    struct IndexTuple
    {
    };

    template<size_t I, typename IndexTuple, typename... Types>
    struct MakeIndexTupleImpl;

    template<size_t I, size_t... Indexes, typename T, typename ... Types>
    struct MakeIndexTupleImpl<I, IndexTuple<Indexes...>, T, Types...>
    {
        using Type = typename MakeIndexTupleImpl<I + 1, IndexTuple<Indexes..., I>, Types...>::Type;
    };

    template<size_t I, size_t... Indexes>
    struct MakeIndexTupleImpl<I, IndexTuple<Indexes...> >
    {
        using Type = IndexTuple<Indexes...>;
    };

    template<typename... Types>
    struct MakeIndexTuple : MakeIndexTupleImpl<0, IndexTuple<>, Types...>
    {
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    
    template<std::size_t...>
    struct Indices
    {
    };
    
    template<std::size_t N, std::size_t... Is>
    struct MakeIndices : MakeIndices<N-1, N-1, Is...> {};
    
    template<std::size_t... Is>
    struct MakeIndices<0, Is...>
    {
        using Type = Indices<Is...>;
    };

    template<typename T>
    using RemoveCVR = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    template<typename T>
    struct ValueTraits;

    template<typename T>
    struct ValueTraits<T&> : ValueTraits<RemoveCVR<T>>
    {
    };

    template<typename T>
    struct ValueTraits<T&&> : ValueTraits<RemoveCVR<T>>
    {
    };

    template<typename T>
    struct IntValueTraits
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        static_assert(std::is_signed<T>::value, "T must be signed");
        static_assert(!std::is_floating_point<T>::value, "T must not be floating point");

        static inline T read(lua_State* luaState, int index) noexcept
        {
            return static_cast<T>( lua_tointeger(luaState, index) );
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            if (!lua_isnumber(luaState, index))
                return false;

            const auto eps = std::numeric_limits<T>::epsilon();
            const auto min = std::numeric_limits<T>::min();
            const auto max = std::numeric_limits<T>::max();
            Number number = lua_tonumber(luaState, index);
            if(number < min || number > max)
                return false;

            return std::abs(number - static_cast<T>(number + eps)) <= eps;
        }

        static inline void get(lua_State* luaState, int index, T key) noexcept
        {
            lua_rawgeti(luaState, index, key);
        }

        static inline int push(lua_State* luaState, T value) noexcept
        {
            lua_pushinteger(luaState, value);
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

    template<typename T>
    struct UIntValueTraits
    {
        static_assert(std::is_integral<T>::value, "T must be integral");
        static_assert(!std::is_signed<T>::value, "T must not be signed");
        static_assert(!std::is_floating_point<T>::value, "T must not be floating point");

        static inline T read(lua_State* luaState, int index) noexcept
        {
            return static_cast<T>( lua_tointeger(luaState, index) );
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            if (!lua_isnumber(luaState, index))
                return false;

            const auto eps = std::numeric_limits<T>::epsilon();
            const auto max = std::numeric_limits<T>::max();
            Number number = lua_tonumber(luaState, index);
            if(number < 0 || number > max)
                return false;

            return std::abs(number - static_cast<T>(number + eps)) <= eps;
        }

        static inline void get(lua_State* luaState, int index, T key) noexcept
        {
            lua_rawgeti(luaState, index, key);
        }

        static inline int push(lua_State* luaState, T value) noexcept
        {
            lua_pushinteger(luaState, static_cast<typename std::make_signed<T>::type>(value));
            return 1;
        }
    };

    template<>
    struct ValueTraits<unsigned long long> : public UIntValueTraits<unsigned long long>
    {
    };

    template<>
    struct ValueTraits<unsigned long> : public UIntValueTraits<unsigned long>
    {
    };

    template<>
    struct ValueTraits<unsigned int> : public UIntValueTraits<unsigned int>
    {
    };

    template<>
    struct ValueTraits<unsigned short> : public UIntValueTraits<unsigned short>
    {
    };

    template<>
    struct ValueTraits<unsigned char> : public UIntValueTraits<unsigned char>
    {
    };

    template<>
    struct ValueTraits<lua::Boolean>
    {
        static inline lua::Boolean read(lua_State* luaState, int index) noexcept
        {
            return lua_toboolean(luaState, index) != 0;
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            return lua_isboolean(luaState, index) != 0;
        }

        static inline void get(lua_State* luaState, int index, lua::Boolean key) noexcept
        {
            lua_rawgeti(luaState, index, key);
        }

        static inline int push(lua_State* luaState, lua::Boolean value) noexcept
        {
            lua_pushboolean(luaState, value);
            return 1;
        }
    };

    template<typename T>
    struct FloatValueTraits
    {
        static_assert(std::is_floating_point<T>::value, "T must not be floating point");

        static inline T read(lua_State* luaState, int index) noexcept
        {
            return static_cast<T>( lua_tonumber(luaState, index) );
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            return lua_isnumber(luaState, index) != 0;
        }

        static inline int push(lua_State* luaState, T value) noexcept
        {
            lua_pushnumber(luaState, value);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::Number> : public FloatValueTraits<lua::Number>
    {
    };

    template<>
    struct ValueTraits<float> : public FloatValueTraits<float>
    {
    };

    template<>
    struct ValueTraits<long double> : public FloatValueTraits<long double>
    {
    };

    template<>
    struct ValueTraits<lua::String>
    {
        static inline lua::String read(lua_State* luaState, int index) noexcept
        {
            return lua_tostring(luaState, index);
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            // Lua is treating numbers also like strings, because they are always convertible to string
            if (lua_isnumber(luaState, index))
                return false;

            return lua_isstring(luaState, index) != 0;
        }

        static inline void get(lua_State* luaState, int index, lua::String key) noexcept
        {
            lua_getfield(luaState, index, key);
        }

        static inline int push(lua_State* luaState, lua::String value) noexcept
        {
            lua_pushstring(luaState, value);
            return 1;
        }

        static inline int push(lua_State* luaState, lua::String value, std::size_t length) noexcept
        {
            lua_pushlstring(luaState, value, length);
            return 1;
        }
    };

    template<>
    struct ValueTraits<std::string>
    {
        static inline std::string read(lua_State* luaState, int index) noexcept
        {
            lua::String str = lua_tostring(luaState, index);
            return str==nullptr ? std::string() : str;
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            // Lua is treating numbers also like strings, because they are always convertible to string
            if (lua_isnumber(luaState, index))
                return false;

            return lua_isstring(luaState, index) != 0;
        }

        static inline void get(lua_State* luaState, int index, const std::string& key) noexcept
        {
            lua_getfield(luaState, index, key.c_str());
        }

        static inline int push(lua_State* luaState, const std::string& value) noexcept
        {
            lua_pushstring(luaState, value.c_str());
            return 1;
        }

        static inline int push(lua_State* luaState, const std::string& value, std::size_t length) noexcept
        {
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
        static inline lua::Nil read(lua_State*, int) noexcept
        {
            return nullptr;
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            return lua_isnoneornil(luaState, index) != 0;
        }

        static inline int push(lua_State* luaState, lua::Nil) noexcept
        {
            lua_pushnil(luaState);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::Pointer>
    {
        static inline lua::Pointer read(lua_State* luaState, int index) noexcept
        {
            return lua_touserdata(luaState, index);
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            return lua_islightuserdata(luaState, index) != 0;
        }

        static inline int push(lua_State* luaState, lua::Pointer value) noexcept
        {
            lua_pushlightuserdata(luaState, value);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::Table>
    {
        static inline lua::Table read(lua_State*, int) noexcept
        {
            return {};
        }

        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            return lua_istable(luaState, index) != 0;
        }

        static inline void get(lua_State* luaState, int index, lua::Table) noexcept
        {
            lua_gettable(luaState, index);
        }

        static inline int push(lua_State* luaState, lua::Table) noexcept
        {
            lua_newtable(luaState);
            return 1;
        }
    };

    template<>
    struct ValueTraits<lua::Callable>
    {
        static inline bool isCompatible(lua_State* luaState, int index) noexcept
        {
            bool isCallable = lua_isfunction(luaState, index) != 0 || lua_iscfunction(luaState, index) != 0;

            if (!isCallable)
            {
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
        static int pushTuple(lua_State* luaState, traits::IndexTuple<Indexes...>, std::tuple<Args&&...>&& tup) noexcept
        {
            return pushRec(luaState, std::get<Indexes>(tup)...);
        }

        template<std::size_t... Indexes>
        static int pushTuple(lua_State* luaState, traits::IndexTuple<Indexes...>, const std::tuple<Args...>& tup) noexcept
        {
            return pushRec(luaState, std::get<Indexes>(tup)...);
        }

        template<typename T1, typename... Ts>
        static int pushRec(lua_State* luaState, T1&& value1,  Ts&&... values) noexcept
        {
            auto n = ValueTraits<T1>::push(luaState, std::forward<T1>(value1));
            n += pushRec(luaState, std::forward<Ts>(values)...);
            return n;
        }

        static inline int pushRec(lua_State*) noexcept
        {
            return 0;
        }

    public:
        static inline int push(lua_State* luaState, std::tuple<Args&&...>&& value) noexcept
        {
            return pushTuple(luaState, typename traits::MakeIndexTuple<Args...>::Type(), std::forward<std::tuple<Args&&...>>(value));
        }

        static inline int push(lua_State* luaState, const std::tuple<Args...>& value) noexcept
        {
            return pushTuple(luaState, typename traits::MakeIndexTuple<Args...>::Type(), value);
        }

        static inline int push(lua_State* luaState, Args&&... args) noexcept
        {
            return pushRec(luaState, std::forward<Args>(args)...);
        }
    };
}}
