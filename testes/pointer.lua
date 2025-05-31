print("testing pointer")

local r, s
do
	local t = { "a", "b", "c" }
	r = ptr.addr(t, 2)
	assert(tostring(r) == string.format("pointer: %p", r))
	assert(r[nil] == "b")

	r[nil] = "d"
	assert(r[nil] == "d")
	assert(t[2] == "d")

	--[[
	t[100] = "e"
	assert(t[100] == "e")
	print(ptr.addr(t, 100), r)
	assert(tostring(r) == string.format("pointer: %p", r))
	assert(r[nil] == "d")
	]]

	t.f = "g"
	s = ptr.addr(t, "f")
	assert(s[nil] == "g")
	assert(t.f == "g")

	s[nil] = "h"
	assert(s[nil] == "h")
	assert(t.f == "h")
end

collectgarbage()

assert(r[nil] == "d")

print "OK"
