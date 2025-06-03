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

#define MINLZTABSIZE ((MINSTRTABSIZE < 4) ? 4 : MINSTRTABSIZE)

void luaA_init (lua_State *L) {
	lzarray *f = &G(L)->lzfree;
	f->array = luaM_newvector(L, MINLZTABSIZE , lznode);
	lua_assert(f->array != NULL);
	f->size = MINLZTABSIZE;
	f->nitems = 0;
}

void luaA_collect (lua_State *L) {
	lzarray *f = &G(L)->lzfree;
	size_t i, j;
	StkId p;

	for (p = L->stack.p; p != L->stack_last.p; ++p) {
		TValue *addr = s2v(p);
		if (ttisaddress(addr)) {
			lua_Ptr ptr = luaA_revive(avalue(addr));
			setavalue(addr, ptr);
		}
	}

	for (i = j = 0; i != f->nitems; ++i) {
		if (f->array[i].rc == 0)
			luaM_freearray(L, f->array[i].mem, f->array[i].size);
		else
			f->array[j++] = f->array[i];
	}
	f->nitems = j;
}


/* Should only be called if t->rc != 0 */
static inline void luaA_lzfree (lua_State *L, lznode node) {
	lzarray *f = &G(L)->lzfree;
	if (node.mem == NULL)
		return;
	else if (f->nitems >= f->size) {
		size_t newsize = f->size;
		lua_assert(newsize >= 4);
		while (f->nitems >= newsize)
			newsize = (newsize / 2) * 3;
		f->array = luaM_reallocvector(L, f->array, f->size, newsize, lznode);
		lua_assert(f->array != NULL);
		f->size = newsize;
	}
	f->array[f->nitems] = node;
	f->nitems += 1;
}


void luaA_freearray (lua_State *L, TValue *array, size_t size) {
	lznode node;
	node.mem = array;
	node.size = size;
	node.rc = 1;
	luaA_lzfree(L, node);
}


/* Should only be called if t->rc != 0 */
TValue *luaA_reallocarray (lua_State *L, TValue *array, size_t oldsize, size_t size) {
	TValue *newarray;
	size_t i = 0;
	lznode node;
	node.mem = array;
	node.size = oldsize;
	node.rc = 0;

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
	luaA_lzfree(L, node);
	return newarray;
}


/* Address of rawget(t, key) */
lua_Ptr luaA_addr (lua_State *L, Table *t, const TValue *key) {
	(void) L;
	t->rc |= 1;
	return (TValue *) luaH_get(t, key);
}


lua_Ptr luaA_revive (lua_Ptr ptr) {
	while (ttisforward(ptr))
		ptr = avalue(ptr);
	lua_assert(refcount(ptr) != 0);
	return ptr;
}

const TValue *luaA_deref (lua_State *L, TValue *addr) {
	lua_Ptr ptr = luaA_revive(avalue(addr));
	(void) L;
	setavalue(addr, ptr);
	return ptr;
}

const TValue *luaA_assign (lua_State *L, TValue *addr, const TValue *val) {
	lua_Ptr ptr = luaA_revive(avalue(addr));
	setavalue(addr, ptr);
	setobj(L, ptr, val);
	return val;
}
