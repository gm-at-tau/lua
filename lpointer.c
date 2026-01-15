/*
** $Id: lpointer.c $
** LZ Pointer
** Copyright (C) 2025 Gabriel Domingues <gm@mail.tau.ac.il>
** See LUA Copyright Notice in lua.h
*/

#define lpointer_c
#define LUA_CORE

#include "lprefix.h"

#include <stddef.h>

#include "llimits.h"
#include "lua.h"

#include "ldo.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lpointer.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"


void luaA_init (lua_State *L) {
  LZarray *f = &G(L)->lzfree;
  f->arr = NULL;
  f->size = 0;
  f->n = 0;
}


void luaA_collect (lua_State *L) {
  LZarray *f = &G(L)->lzfree;
  int i = 0;
  StkId p = L->stack.p;
  if (p != NULL) {
    for (; p != L->stack_last.p; ++p) {
      TValue *addr = s2v(p);
      if (ttisaddress(addr)) {
        lua_Ptr ptr = luaA_revive(avalue(addr));
        setavalue(addr, ptr);
      }
    }
  }
  for (i = 0; i != f->n; ++i)
    luaM_freemem(L, f->arr[i].mem, f->arr[i].size);
  f->n = 0;
}


void luaA_freemem (lua_State *L, void *array, size_t size) {
  LZarray *f = &G(L)->lzfree;
  if (array == NULL)
    return;
  luaM_growvector(L, f->arr, f->n, f->size, LZnode,
		  SHRT_MAX, "lazy free list");
  f->arr[f->n].mem = array;
  f->arr[f->n].size = size;
  f->n += 1;
}

/* called after atomic(L) */
void luaA_box (lua_State *L, TValue *value) {
  if (isref(value) && isdead(G(L), value)) {
    GCBox *box = luaS_newbox(L, value);
    setavalue(value, boxedvalue(box));
    settt_(value, LUA_VFWDADDRESS);
  } else
    setempty(value);
}

void luaA_forward (TValue *value, TValue *newplace) {
  if (isref(value)) {
    setavalue(value, newplace);
    settt_(value, LUA_VFWDADDRESS);
  } else {
    setempty(value);
  }
}

/* Should only be called if anyref(t) */
TValue *luaA_reallocarray (lua_State *L, TValue *array, size_t oldsize,
                           size_t newsize) {
  size_t i;
  size_t n = (newsize < oldsize) ? newsize : oldsize;
  TValue *newarray = luaM_newvector(L, newsize, TValue);
  if (newarray == NULL && newsize > 0)
    return NULL;
  for (i = 0; i != n; ++i) {
    TValue *value = &array[i];
    newarray[i] = array[i];
    luaA_forward(value, &newarray[i]);
  }
  luaA_freearray(L, array, oldsize);
  return newarray;
}


/* Address of rawget(t, key) */
lua_Ptr luaA_addr (lua_State *L, Table *t, const TValue *key) {
  lua_Ptr ptr = cast(TValue *, luaH_get(t, key));
  UNUSED(L);
  if (!isabstkey(ptr) && !isref(ptr))
    incref(t);
  return ptr;
}


lua_Ptr luaA_revive (lua_Ptr ptr) {
  while (ttisforward(ptr))
    ptr = avalue(ptr);
  lua_assert(isref(ptr));
  return ptr;
}


const TValue *luaA_deref (TValue *addr) {
  lua_Ptr ptr = luaA_revive(avalue(addr));
  setavalue(addr, ptr);
  return ptr;
}


void luaA_assign (lua_State *L, TValue *addr, const TValue *val) {
  lua_Ptr ptr = luaA_revive(avalue(addr));
  setavalue(addr, ptr);
  setobj(L, ptr, val);
}
