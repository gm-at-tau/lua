#!../lua
-- $Id: testes/proc.lua $

local i = 0
proc(function()
	i = i + 1
	print(i, "proc")
	assert(i == 3)
end)

i = i + 1
print(i, "main")
assert(i == 1)
yield()
i = i + 1
print(i, "exit")
assert(i == 2)
