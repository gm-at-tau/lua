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

#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lpointer.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"


#define MINLZTABSIZE ((MINSTRTABSIZE < 4) ? 4 : MINSTRTABSIZE)


void luaA_init (lua_State *L) {
  lzarray *f = &G(L)->lzfree;
  f->array = luaM_newvector(L, MINLZTABSIZE, lznode);
  lua_assert(f->array != NULL);
  f->size = MINLZTABSIZE;
  f->nitems = 0;
}


void luaA_collect (lua_State *L) {
  lzarray *f = &G(L)->lzfree;
  size_t i = 0;
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
  for (i = 0; i != f->nitems; ++i)
    luaM_freearray(L, f->array[i].mem, f->array[i].size);
  f->nitems = 0;
}


/* Should only be called if anyref(t) */
static inline void luaA_lzfree (lua_State *L, TValue *array, size_t size) {
  lzarray *f = &G(L)->lzfree;
  if (array == NULL)
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
  f->array[f->nitems].mem = array;
  f->array[f->nitems].size = size;
  f->nitems += 1;
}


void luaA_freearray (lua_State *L, TValue *array, size_t size) {
  size_t i = 0;
  for (i = 0; i != size; ++i) {
    if (reftype(&array[i]) & BIT_REF) {
      GCBox *box = luaS_newbox(L, &array[i]);
      setavalue(&array[i], boxedvalue(box));
      settt_(&array[i], LUA_VFWDADDRESS);
    } else {
      setempty(&array[i]);
    }
  }
  luaA_lzfree(L, array, size);
}


/* Should only be called if anyref(t) */
TValue *luaA_reallocarray (lua_State *L, TValue *array, size_t oldsize, size_t size) {
  TValue *newarray = luaM_newvector(L, size, TValue);
  size_t i = 0;
  for (i = 0; i != oldsize; ++i) {
    newarray[i] = array[i];
    if (reftype(&array[i]) & BIT_REF) {
      setavalue(&array[i], &newarray[i]);
      settt_(&array[i], LUA_VFWDADDRESS);
    } else {
      setempty(&array[i]);
    }
  }
  luaA_lzfree(L, array, oldsize);
  return newarray;
}


/* Address of rawget(t, key) */
lua_Ptr luaA_addr (lua_State *L, Table *t, lua_Integer key) {
  lua_Ptr ptr = (TValue *)luaH_getarray(t, key);
  UNUSED(L);
  if (isempty(ptr))
    return ptr;
  if (!(reftype(ptr) & BIT_REF))
    incref(t);
  return ptr;
}


lua_Ptr luaA_revive (lua_Ptr ptr) {
  while (ttisforward(ptr))
    ptr = avalue(ptr);
  lua_assert(reftype(ptr) & BIT_REF);
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
