//
#ifndef lpointer_h
#define lpointer_h


#include "lobject.h"
#include "lua.h"

LUAI_FUNC void luaA_init (lua_State *L);

LUAI_FUNC lua_Ptr luaA_addr (lua_State *L, Table *t, const TValue *key);
const TValue *luaA_deref (lua_State *L, lua_Ptr addr);
const TValue *luaA_assign (lua_State *L, TValue *addr, const TValue *val);

#endif
