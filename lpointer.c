//
#define lpointer_c
#define LUA_CORE

#include "lprefix.h"

#include <stddef.h>

#include "lua.h"

#include "lobject.h"
#include "lpointer.h"
#include "ltable.h"
#include "lgc.h"

void luaA_init (lua_State *L) {
	(void) L;
}

lua_Ptr luaA_addr (lua_State *L, Table *t, const TValue *key) {
	(void) L;
	return (TValue *) luaH_get(t, key);
}

const TValue *luaA_deref (lua_State *L, lua_Ptr addr) {
	(void) L;
	addr = avalue(addr);
	lua_assert(refcount(addr) != 0);
	return addr;
}

const TValue *luaA_assign (lua_State *L, TValue *addr, const TValue *val) {
	lua_Ptr ptr = avalue(addr);
	lua_assert(refcount(ptr) != 0);
	setobj(L, ptr, val);
	return val;
}
