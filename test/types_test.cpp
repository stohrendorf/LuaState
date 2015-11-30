//
//  state_tests.h
//  LuaState
//
//  Created by Simon Mikuda on 16/04/14.
//
//  See LICENSE and README.md files

#include "test.h"

//////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    // We create state and variables
    lua::State state;
    state.doString(createVariables);
    
    // Boolean
    bool boolValue = false;
    
    assert(state["boolean"].toBool() == true);
    assert((boolValue = state["boolean"].toBool()) == true);
    
    // All numeric types
    int                 intValue;
    long                longValue;
    long long           longlongValue;
    unsigned            unsignedValue;
    long unsigned       unsignedLongValue;
    long long unsigned  unsignedLongLongValue;
    short               shortValue;
    unsigned short      unsignedShortValue;
    
    state.set("value", 1);
    assert(state["value"].toInt() == 1);
    state.set("value", 1L);
    assert(state["value"].toInt() == 1);
    state.set("value", 1LL);
    assert(state["value"].toInt() == 1);
    state.set("value", static_cast<short>(1));
    assert(state["value"].toInt() == 1);
    state.set("value", static_cast<signed char>(1));
    assert(state["value"].toInt() == 1);

    assert(state["integer"].toInt() == 10);
    
    state.set("value", 1.0);
    assert(state["value"].toNumber() == 1);
    state.set("value", 1.f);
    assert(state["value"].toNumber() == 1);
    state.set("value", 1.l);
    assert(state["value"].toNumber() == 1);
    
    assert(state["number"].toNumber() == 2.5);
    
    // All character types
    char             charValue;
    const char*            charString;

    state.set("value", "x");
    assert(state["value"].toCStr()[0] == 'x');
    state.set("value", "ahoj");
    assert(strcmp(state["value"].toCStr(), "ahoj") == 0);
    
    assert(state["char"].toCStr()[0] == 'a');
    assert(state["char"].toCStr()[0] != 'b');
    assert((charValue = state["char"].toCStr()[0]) == 'a');
    assert((charValue = state["char"].toCStr()[0]) != 'b');

    assert(strcmp(state["text"].toCStr(), "hello") == 0);
    assert(strcmp(state["text"].toCStr(), "bannana") != 0);
    assert(strcmp(charString = state["text"].toCStr(), "hello") == 0);
    assert(strcmp(charString = state["text"].toCStr(), "bannana") != 0);
    
    std::string stringValue = "test string";
    state.set("value", stringValue);
    assert(state["value"].toString() == stringValue);
    assert((stringValue = state["text"].toString()) == "hello");
    assert((stringValue = state["text"].toString()) != "bannana");
    
    char binaryData[4]{"abc"};
    state.setData("binary", binaryData, 3);
    assert(std::strcmp(state["binary"].toCStr(), "abc") == 0);
    
    state.checkMemLeaks();
    return 0;
}
