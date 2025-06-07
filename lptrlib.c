/*
** $Id: lptrlib.c $
** Indexed Pointer Library
** Copyright (C) 2025 Gabriel Domingues <gm@mail.tau.ac.il>
** See LUA Copyright Notice in lua.h
*/
#define lptrlib_c
#define LUA_LIB

#include <stddef.h>

#include "lua.h"
#include "luaconf.h"

#include "lauxlib.h"
#include "lgc.h"
#include "lobject.h"
#include "lualib.h"


#define POINTER_METATABLE "pointer"


static void pushpointer (lua_State *L, int idx) {
  luaL_checkudata(L, idx, POINTER_METATABLE);
  if (lua_getiuservalue(L, idx, 1) != LUA_TTABLE)
    luaL_error(L, "corrupted pointer");
  if (lua_getiuservalue(L, idx - (idx <= 0), 2) == LUA_TNONE)
    luaL_error(L, "corrupted pointer");
}


static int pointer_addr (lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  lua_newuserdatauv(L, 0, 2);

  lua_pushvalue(L, 1);
  lua_setiuservalue(L, -2, 1);

  lua_pushvalue(L, 2);
  lua_setiuservalue(L, -2, 2);

  luaL_getmetatable(L, POINTER_METATABLE);
  lua_setmetatable(L, -2);

  return 1;
}


static int pointer_tostring (lua_State *L) {
  pushpointer(L, 1);
  lua_pushfstring(L, "%p[%s]", lua_topointer(L, -2), lua_tostring(L, -1));
  return 1;
}


static int pointer_deref (lua_State *L) {
  pushpointer(L, 1);
  lua_gettable(L, -2);
  return 1;
}


static int pointer_assign (lua_State *L) {
  pushpointer(L, 1);
  lua_pushvalue(L, 2);
  lua_settable(L, -3);
  return 1;
}


static const luaL_Reg pointer_methods[] = {
    {"deref", pointer_deref},
    {"assign", pointer_assign},
    {NULL, NULL},
};


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


static int pointer_equal (lua_State *L) {
  pushpointer(L, 1);
  pushpointer(L, 2);
  if (!lua_compare(L, -4, -2, LUA_OPEQ))
    return 0;
  if (!lua_compare(L, -3, -1, LUA_OPEQ))
    return 0;
  return 1;
}


static const luaL_Reg pointer_metamethods[] = {
  {"__tostring", pointer_tostring},
  {"__index", pointer_index},
  {"__newindex", pointer_newindex},
  {"__eq", pointer_equal},
  {NULL, NULL},
};


static const luaL_Reg pointer_functions[] = {
  {"addr", pointer_addr},
  {"deref", pointer_deref},
  {"assign", pointer_assign},
  {"index", pointer_index},
  {"newindex", pointer_newindex},
  {NULL, NULL},
};


LUAMOD_API int luaopen_pointer (lua_State *L) {
  luaL_newmetatable(L, POINTER_METATABLE);

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  luaL_setfuncs(L, pointer_metamethods, 0);
  luaL_setfuncs(L, pointer_methods, 0);
  luaL_newlib(L, pointer_functions);

  return 1;
}
