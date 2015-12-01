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
    lua::State state;
    state.doString(createVariables);

    lua::Ref ref{ state["table"]["a"] };
    lua::Ref tabRef{ state["table"] };
    
    assert(ref.unref().toCStr()[0] == 'a');
    assert(tabRef.unref()["a"].toCStr()[0] == 'a');
    
    lua::Ref copyRef = ref;
    assert(copyRef.unref().toCStr()[0] == 'a');
    
    copyRef = tabRef;
    assert(copyRef.unref()["a"].toCStr()[0] == 'a');
        
    state.checkMemLeaks();
    return 0;
}
