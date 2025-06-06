print("testing pointer")
local gc = { "incremental", "generational" }

for _, opt in ipairs(gc) do
	print(opt)
	collectgarbage(opt)

	local refs = {}
	local f, r, s
	do
		local t = { "a", "b", "c" }
		r = ptr.addr(t, 2)
		assert(tostring(r) == string.format("pointer: %p", r))
		assert(r[nil] == "b")

		r[nil] = "d"
		assert(r[nil] == "d")
		assert(t[2] == "d")

		t[100] = "e"
		assert(t[100] == "e")
		assert(r[nil] == "d")
		s = ptr.addr(t, 2)
		assert(r == s)

		t[2] = "x"
		assert(t[2] == "x")
		assert(r[nil] == "x")

		--[[
		t.f = "g"
		f = ptr.addr(t, "f")
		assert(f[nil] == "g")
		assert(t.f == "g")

		f[nil] = "h"
		assert(f[nil] == "h")
		assert(t.f == "h")
		]]

		assert(not pcall(function()
			local a = {}
			a[r] = 3
			return a
		end))

		t = nil
	end
	refs[1] = r
	refs[2] = s
	print "SCOPE"

	local box = (function()
		local t = { 42 }
		return ptr.addr(t, 1)
	end)()

	-- once for table, once for box
	for i = 1, 2 do
		collectgarbage()

		assert(tostring(r) == tostring(s))
		assert(r == refs[1])
		assert(r == refs[2])
		assert(r[nil] == "x")
		assert(s[nil] == "x")
		-- assert(f[nil] == "h")
	end
	print "OK"
end

print "OK"
