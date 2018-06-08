-- This file was automatically generated for the LuaDist project.

package = "lua-mtrace"
version = "0.1-1"
-- LuaDist source
source = {
  tag = "0.1-1",
  url = "git://github.com/LuaDist-testing/lua-mtrace.git"
}
-- Original source
-- source = {
--    url = "hg+https://code.zash.se/lua-mtrace/";
--    tag = "0.1";
-- }
description = {
   homepage = "https://code.zash.se/lua-mtrace/",
   license = "MIT"
}
dependencies = {}
build = {
   type = "builtin",
   modules = {
      mtrace = {
         sources = "mtrace.c"
      }
   }
}