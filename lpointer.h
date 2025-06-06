//
#ifndef lpointer_h
#define lpointer_h


#include "lobject.h"
#include "lua.h"

LUAI_FUNC void luaA_init (lua_State *L);
LUAI_FUNC void luaA_collect (lua_State *L);

LUAI_FUNC TValue *luaA_reallocarray (lua_State *L, TValue *array, size_t oldsize, size_t size);
LUAI_FUNC void luaA_freearray (lua_State *L, TValue *array, size_t size);

LUAI_FUNC lua_Ptr luaA_addr (lua_State *L, Table *t, const TValue *key);
LUAI_FUNC lua_Ptr luaA_revive (lua_Ptr addr);
LUAI_FUNC const TValue *luaA_deref (lua_Ptr addr);
LUAI_FUNC const TValue *luaA_assign (lua_State *L, TValue *addr, const TValue *val);

#endif
