//
#define lpointer_c
#define LUA_CORE

#include "lprefix.h"

#include <stddef.h>

#include "llimits.h"
#include "lua.h"

#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lpointer.h"
#include "lstate.h"
#include "ltable.h"

void luaA_init (lua_State *L) {
	freetable *f = &G(L)->freetbl;
	size_t i = 0;
	f->array = luaM_newvector(L, MINSTRTABSIZE, freenode);
	lua_assert(f->array != NULL);
	f->size = MINSTRTABSIZE;
	f->nitems = 0;
	for (i = 0; i != MINSTRTABSIZE; ++i) {
		f->array[i].mem = NULL;
		f->array[i].size = 0;
	}
}


static l_inline void luaA_free (lua_State *L, TValue *array, size_t size) {
	freetable *f = &G(L)->freetbl;
	if (f->nitems > f->size) {
		size_t newsize = f->size;
		lua_assert(newsize >= 1);
		while (f->nitems > newsize)
			newsize = (newsize / 2) * 3;
		f->array = luaM_reallocvector(L, f->array, f->size, newsize, freenode);
		lua_assert(f->array != NULL);
		f->size = newsize;
	}
	f->array[f->nitems].mem = array;
	f->array[f->nitems].size = size;
	f->nitems += 1;
}


/* Should only be called if t->rc != 0 */
TValue *luaA_reallocarray (lua_State *L, TValue *array, size_t oldsize, size_t size) {
	TValue *newarray;
	size_t i = 0;
	newarray = luaM_newvector(L, size, TValue);
	for (i = 0; i != oldsize; ++i) {
		newarray[i] = array[i];
		if (refcount(&array[i]) == 0) {
			setempty(&array[i]);
		} else {
			setavalue(&array[i], &newarray[i]);
			settt_(&array[i], LUA_VFWDADDRESS);
		}
	}
	luaA_free(L, array, oldsize);
	return newarray;
}


/* Address of rawget(t, key) */
lua_Ptr luaA_addr (lua_State *L, Table *t, const TValue *key) {
	(void) L;
	t->rc |= 1;
	return (TValue *) luaH_get(t, key);
}


lua_Ptr luaA_revive (lua_State *L, lua_Ptr ptr) {
	(void) L;
	while (ttisforward(ptr))
		ptr = avalue(ptr);
	lua_assert(refcount(ptr) != 0);
	return ptr;
}

const TValue *luaA_deref (lua_State *L, TValue *addr) {
	lua_Ptr ptr = luaA_revive(L, avalue(addr));
	setavalue(addr, ptr);
	return ptr;
}

const TValue *luaA_assign (lua_State *L, TValue *addr, const TValue *val) {
	lua_Ptr ptr = luaA_revive(L, avalue(addr));
	setavalue(addr, ptr);
	setobj(L, ptr, val);
	return val;
}
