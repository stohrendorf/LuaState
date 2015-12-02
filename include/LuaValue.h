//
//  LuaValue.h
//  LuaState
//
//  Created by Simon Mikuda on 17/03/14.
//
//  See LICENSE and README.md files

#pragma once

#include "LuaException.h"
#include "LuaPrimitives.h"
#include "LuaStack.h"
#include "LuaStackItem.h"

#include <cassert>
#include <memory>

namespace lua {
    
    class Value;
    class State;
    class ValueReference;
    template<typename... Ts> class Return;

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// This is class for:
    /// * querying values from lua tables,
    /// * setting values to lua tables,
    /// * calling values as functions,
    /// * checking value type.
    class Value
    {
        friend class State;
        friend class ValueReference;
        template <typename... Ts> friend class Return;
        
        std::shared_ptr<detail::StackItem> m_stack = nullptr;
        
        /// Constructor for lua::State class. Whill get global in _G table with name
        ///
        /// @param luaState     Pointer of Lua state
        /// @param deallocQueue Queue for deletion values initialized from given luaState
        /// @param name         Key of global value
        Value(lua_State* luaState, detail::DeallocQueue* deallocQueue, const char* name)
            : m_stack(std::make_shared<detail::StackItem>(luaState, deallocQueue, lua_gettop(luaState), 1, 0))
        {
            lua_getglobal(m_stack->state, name);
        }
        
        template<typename... Ts>
        void callFunction(bool protectedCall, Ts&&... args) const
        {
            constexpr size_t argCount = 1 + sizeof...(args);
            
            // Function must be on top of stack
            assert(traits::ValueTraits<lua::Callable>::isCompatible(m_stack->state, lua_gettop(m_stack->state)));
            
            traits::ValueTraits<std::tuple<Ts...>>::push(m_stack->state, std::forward<Ts>(args)...);

            if (protectedCall)
            {
                if (lua_pcall(m_stack->state, argCount, LUA_MULTRET, 0))
                    throw RuntimeError(m_stack->state);
            }
            else
            {
                lua_call(m_stack->state, argCount, LUA_MULTRET);
            }
        }
        
        template<typename... Ts>
        Value executeFunction(bool protectedCall, Ts&&... args) const
        {
            
            int stackTop = lua_gettop(m_stack->state);
            
            // We will duplicate Lua function value, because it will get poped from stack
            lua_pushvalue(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped);
            
            callFunction(protectedCall, std::forward<Ts>(args)...);
            int returnedValues = lua_gettop(m_stack->state) - stackTop;
            
            assert(returnedValues >= 0);
            
            return Value(std::make_shared<detail::StackItem>(m_stack->state, m_stack->deallocQueue, stackTop, returnedValues, returnedValues == 0 ? 0 : returnedValues - 1));
        }
        
        template<typename... Ts>
        Value&& executeFunction(bool protectedCall, Ts&&... args)
        {
            
            int stackTop = lua_gettop(m_stack->state);
            
            // we check if there are not pushed values before function
            if (m_stack->top + m_stack->pushed < stackTop)
            {
                
                m_stack->deallocQueue->push(detail::DeallocStackItem(m_stack->top, m_stack->pushed));

                lua_pushvalue(m_stack->state, m_stack->top + 1);
                
                m_stack->top = stackTop;
                m_stack->pushed = 1;
                m_stack->grouped = 0;
                
                ++stackTop;
            }
            
            // StackItem top must same as top of current stack
            assert(m_stack->top + m_stack->pushed == lua_gettop(m_stack->state));
            
            callFunction(protectedCall, std::forward<Ts>(args)...);

            m_stack->grouped = lua_gettop(m_stack->state) - stackTop;
            m_stack->pushed += m_stack->grouped;

            assert(m_stack->pushed >= 0);
            
            return std::move(*this);
        }
        
    public:
        
        /// Enable to initialize empty Value, so we can set it up later
        Value() = default;

        /// Constructor for returning values from functions and for creating lua::Ref instances
        ///
        /// @param stackItem Prepared stack item
        Value(std::shared_ptr<detail::StackItem>&& stackItem)
            : m_stack(std::move(stackItem))
        {
        }
        
        /// With this function we will create lua::Ref instance
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value operator[](T&& key) const {
            traits::ValueTraits<T>::get(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped, std::forward<T>(key));
            return Value(std::make_shared<detail::StackItem>(m_stack->state, m_stack->deallocQueue, lua_gettop(m_stack->state) - 1, 1, 0));
        }
        
        /// While chaining [] operators we will call this function multiple times and can query nested tables.
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename T>
        Value&& operator[](T&& key) &&
        {
            traits::ValueTraits<T>::get(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped, std::forward<T>(key));
            ++m_stack->pushed;

            m_stack->grouped = 0;

            return std::forward<Value>(*this);
        }

        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename... Ts>
        Value operator()(Ts&&... args) const
        {
            return  executeFunction(false, std::forward<Ts>(args)...);
        }
        
        /// Protected call of given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename... Ts>
        Value call(Ts&&... args) const
        {
            return executeFunction(true, std::forward<Ts>(args)...);
        }
        
        /// Call given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename... Ts>
        Value&& operator()(Ts&&... args) &&
        {
            return executeFunction(false, std::forward<Ts>(args)...);
        }
        
        /// Protected call of given value.
        ///
        /// @note This function doesn't check if current value is lua::Callable. You must use is<lua::Callable>() function if you want to be sure
        template<typename... Ts>
        Value&& call(Ts&&... args) &&
        {
            return executeFunction(true, std::forward<Ts>(args)...);
        }
        
        template<typename T>
        T to() const
        {
            return traits::ValueTraits<T>::read(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped);
        }
        
        /// Set values to table to the given key.
        ///
        /// @param key      Key to which value will be stored
        /// @param value    Value to be stored to table on given key
        ///
        /// @note This function doesn't check if current value is lua::Table. You must use is<lua::Table>() function if you want to be sure
        template<typename K, typename T>
        void set(K&& key, T&& value) const {
            traits::ValueTraits<K>::push(m_stack->state, std::forward<K>(key));
            traits::ValueTraits<T>::push(m_stack->state, std::forward<T>(value));
            lua_settable(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped);
        }

        /// Check if queryied value is some type from LuaPrimitives.h file
        ///
        /// @return true if yes false if no
        template <typename T>
        bool is() const
        {
            return traits::ValueTraits<T>::isCompatible(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped);
        }
        
        /// First check if lua::Value is type T and if yes stores it to value
        ///
        /// @param value    Reference to variable where will be stored result if types are right
        ///
        /// @return true if value was given type and stored to value false if not
        template <typename T>
        bool get(T& value) const
        {
            if (!is<T>())
            {
                return false;
            }
            else
            {
                value = traits::ValueTraits<T>::read(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped);
                return true;
            }
        }
            
        /// @returns Value position on stack
        int getStackIndex() const
        {
            assert(m_stack->pushed > 0);
            return m_stack->top + 1;
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////////
        // Conventional conversion functions

        // to
        
        const char* toCStr() const
        {
            return to<const char*>();
        }
        
        std::string toString() const
        {
            return to<lua::String>();
        }
        
        lua::Number toNumber() const
        {
            return to<lua::Number>();
        }
        
        float toFloat() const
        {
            return to<float>();
        }

        lua::Integer toInt() const
        {
            return to<lua::Integer>();
        }
        
        lua::Unsigned toUInt() const
        {
            return to<lua::Unsigned>();
        }

        lua::Boolean toBool() const
        {
            return to<lua::Boolean>();
        }

        /// Will get pointer casted to given template type
        ///
        /// @return Pointer staticaly casted to given template type
        template <typename T>
        T* toPtr() const
        {
            return static_cast<T*>(to<Pointer>());
        }
        
        // get
        
        bool getCStr(const char*& cstr) const
        {
            return get<const char*>(cstr);
        }
        
        bool getString(std::string& string) const
        {
            lua::String cstr;
            bool success = get<lua::String>(cstr);
            
            if (success)
                string = cstr;
            
            return success;
        }
        
        bool getNumber(lua::Number number) const
        {
            return get<lua::Number>(number);
        }
        
        bool getInt(lua::Integer number) const
        {
            return get<lua::Integer>(number);
        }
        
        template <typename T>
        T* getPtr(T*& pointer) const
        {
            lua::Pointer cptr;
            bool success = get<lua::Pointer>(cptr);
            
            if (success)
                pointer = static_cast<T*>(cptr);
            
            return success;
        }
        
        //////////////////////////////////////////////////////////////////////////////////////////////
        // Conventional setting functions
        
        template<typename K>
        void setCStr(K&& key, lua::String value) const
        {
            set<const char*>(std::forward<K>(key), value);
        }
        
        template<typename K>
        void setData(K&& key, lua::String value, size_t length) const
        {
            traits::ValueTraits<K>::push(m_stack->state, std::forward<K>(key));
            traits::ValueTraits<lua::String>::push(m_stack->state, value, length);
            lua_settable(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped);
        }
        
        template<typename K>
        void setString(K&& key, const std::string& string) const
        {
            traits::ValueTraits<K>::push(m_stack->state, std::forward<K>(key));
            traits::ValueTraits<lua::String>::push(m_stack->state, string.c_str(), string.length());
            lua_settable(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped);
        }
        
        size_t length() const
        {
            return lua_rawlen(m_stack->state, m_stack->top + m_stack->pushed - m_stack->grouped);
        }

        template<typename K>
        void set(K&& key, std::string&& value) const
        {
            setString<lua::String>(std::forward<K>(key), std::forward<std::string>(value));
        }
        
        template<typename K>
        void setNumber(K&& key, lua::Number number) const
        {
            set<lua::Number>(std::forward<K>(key), number);
        }
        
        template<typename K>
        void setInt(K&& key, int number) const
        {
            set<int>(std::forward<K>(key), number);
        }
        
        template<typename K>
        void setFloat(K&& key, float number) const
        {
            set<float>(std::forward<K>(key), number);
        }
        
        template<typename K>
        void setDouble(K&& key, double number) const
        {
            set<double>(std::forward<K>(key), number);
        }
    };

    template<>
    inline lua::Value Value::to() const
    {
        return *this;
    }

    namespace traits {
        template<>
        struct ValueTraits<lua::Value>
        {
            static inline int push(lua_State* luaState, lua::Value&& value) {
                lua_pushvalue(luaState, value.getStackIndex());
                return 1;
            }

            static inline int push(lua_State* luaState, const lua::Value& value) {
                lua_pushvalue(luaState, value.getStackIndex());
                return 1;
            }
        };
    }
}

