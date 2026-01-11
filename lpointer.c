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
    luaM_freearray(L, f->arr[i].mem, f->arr[i].size);
  f->n = 0;
}


/* Should only be called if anyref(t) */
static inline void luaA_lzfree (lua_State *L, TValue *array, size_t size) {
  LZarray *f = &G(L)->lzfree;
  if (array == NULL)
    return;
  luaM_growvector(L, f->arr, f->n, f->size, LZnode,
		  SHRT_MAX, "lazy free list");
  f->arr[f->n].mem = array;
  f->arr[f->n].size = size;
  f->n += 1;
}


void luaA_freearray (lua_State *L, TValue *array, size_t size) {
  lu_byte white = otherwhite(G(L)); /* called after atomic(L) */
  size_t i = 0;
  for (i = 0; i != size; ++i) {
    TValue *v = &array[i];
    if (isref(v) && isdeadm(white, v->marked)) {
      GCBox *box = luaS_newbox(L, v);
      setavalue(v, boxedvalue(box));
      settt_(v, LUA_VFWDADDRESS);
    } else {
      setempty(v);
    }
  }
  luaA_lzfree(L, array, size);
}


/* Should only be called if anyref(t) */
TValue *luaA_reallocarray (lua_State *L, TValue *array, size_t oldsize, size_t size) {
  TValue *newarray = luaM_newvector(L, size, TValue);
  size_t i = 0;
  for (i = 0; i != oldsize; ++i) {
    TValue *v = &array[i];
    newarray[i] = array[i];
    if (isref(v)) {
      setavalue(v, &newarray[i]);
      settt_(v, LUA_VFWDADDRESS);
    } else {
      setempty(v);
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
  if (!isref(ptr))
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
