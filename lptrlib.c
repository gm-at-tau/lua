/*
** $Id: lptrlib.c $
** LZ Pointer Library
** Copyright (C) 2025 Gabriel Domingues <gm@mail.tau.ac.il>
** See LUA Copyright Notice in lua.h
*/

#define lptrlib_c
#define LUA_LIB

#include "lprefix.h"

#include <stddef.h>

#include "lua.h"

#include "lapi.h"
#include "lauxlib.h"
#include "lobject.h"
#include "lualib.h"


static int pointer_rawaddr (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);
  lua_rawaddr(L, 1);
  return 1;
}


static int pointer_addr (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_settop(L, 2);
  lua_addr(L, 1);
  return 1;
}


static int pointer_deref (lua_State *L) {
  luaL_checktype(L, 1, LUA_TADDRESS);
  lua_deref(L, 1);
  return 1;
}


static int pointer_assign (lua_State *L) {
  luaL_checktype(L, 1, LUA_TADDRESS);
  lua_settop(L, 2);
  lua_assign(L, 1);
  return 1;
}


/*
** {======================================================
** METAMETHODS
** =======================================================
*/

static int pointer_index (lua_State *L) {
  luaL_checktype(L, 2, LUA_TNIL);
  L->top.p--;
  return pointer_deref(L);
}


static int pointer_newindex (lua_State *L) {
  luaL_checktype(L, 2, LUA_TNIL);
  setobj2s(L, L->top.p - 2, s2v(L->top.p - 1));
  L->top.p--;
  return pointer_assign(L);
}


/* }====================================================== */


static const luaL_Reg pointer_metamethods[] = {
  {"__index", pointer_index},
  {"__newindex", pointer_newindex},
  {"deref", pointer_deref},
  {"assign", pointer_assign},
  {NULL, NULL},
};


static const luaL_Reg pointer_functions[] = {
  {"rawaddr", pointer_rawaddr},
  {"addr", pointer_addr},
  {"deref", pointer_deref},
  {"assign", pointer_assign},
  {"index", pointer_index},
  {"newindex", pointer_newindex},
  {NULL, NULL},
};


/* Similar to lstring.c */
static void createmetatable (lua_State *L) {
  /* table to be metatable for address */
  luaL_newlibtable(L, pointer_metamethods);
  luaL_setfuncs(L, pointer_metamethods, 0);
  {
    lua_lock(L);
    setavalue(s2v(L->top.p), (TValue *)NULL); /* dummy address */
    api_incr_top(L);
    lua_unlock(L);
  }
  lua_pushvalue(L, -2);    /* copy table */
  lua_setmetatable(L, -2); /* set table as metatable for strings */
  lua_pop(L, 2);           /* pop dummy address and metatable */
}


LUAMOD_API int luaopen_pointer (lua_State *L) {
  luaL_newlib(L, pointer_functions);
  createmetatable(L);
  return 1;
}
