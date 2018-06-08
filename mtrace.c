/*
 * Copyright (c) 2017-2018 Kim Alvefur
 *
 * This project is MIT/X11 licensed. Please see the
 * COPYING file in the source package for more information.
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <lua.h>
#include <lauxlib.h>

struct alloc_pair {
	lua_Alloc allocf;
	void *ud;
};

char currentfile[256] = "";

void *tracing_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
	char t = '?';
	FILE *logfile = (FILE *) ud;

	if(ptr == NULL) {
		switch(osize) {
			case LUA_TSTRING:
				t = 's';
				break;

			case LUA_TTABLE:
				t = 't';
				break;

			case LUA_TFUNCTION:
				t = 'f';
				break;

			case LUA_TUSERDATA:
				t = 'u';
				break;

			case LUA_TTHREAD:
				t = 'c'; /* for coroutine */
				break;

			default:
				/* Something else being allocated */
				break;
		}

		osize = 0;
	}

	if(nsize == 0) {
		if(ptr != NULL) {
			fprintf(logfile, "@ %s - %p 0x%zx\n", currentfile, ptr, osize);
		}

		free(ptr);
		return NULL;
	}
	else {
		if(ptr != NULL) {
			fprintf(logfile, "@ %s < %p\n", currentfile, ptr);
		}

		ptr = realloc(ptr, nsize);

		if(osize == 0) {
			fprintf(logfile, "@ %s + %p 0x%zx %c\n", currentfile, ptr, nsize, t);
		}
		else {
			fprintf(logfile, "@ %s > %p 0x%zx\n", currentfile, ptr, nsize);
		}

		return ptr;
	}
}

void linehook(lua_State *L, lua_Debug *ar) {
	lua_getinfo(L, "S", ar);
	snprintf(currentfile, 256, "%s:%d", ar->short_src, ar->currentline);
}

int m_restore_alloc(lua_State *L) {
	struct alloc_pair *old_alloc = lua_touserdata(L, lua_upvalueindex(1));
	lua_setallocf(L, old_alloc->allocf, old_alloc->ud);
	return 0;
}

int m_set_alloc(lua_State *L) {
	snprintf(currentfile, 256, "%s:%d", __FILE__, __LINE__);
	const char *logfilename = luaL_checkstring(L, 1);
	struct alloc_pair *old_alloc = lua_newuserdata(L, sizeof(struct alloc_pair));
	old_alloc->allocf = lua_getallocf(L, &old_alloc->ud);

	FILE *logfile = fopen(logfilename, "a");

	if(logfile == NULL) {
		lua_pushnil(L);
		lua_pushinteger(L, errno);
		return 1;
	}

	lua_setallocf(L, (lua_Alloc)tracing_alloc, (void *)logfile);
	lua_sethook(L, linehook, LUA_MASKLINE, 0);

	lua_pushcclosure(L, m_restore_alloc, 1);
	return 1;
}

int luaopen_mtrace(lua_State *L) {
	lua_pushcfunction(L, m_set_alloc);
	return 1;
}
