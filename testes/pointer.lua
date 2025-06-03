print("testing pointer")
local gc = { "generational", "incremental" }

for _, opt in ipairs(gc) do
	collectgarbage(opt)

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

		t.f = "g"
		f = ptr.addr(t, "f")
		assert(f[nil] == "g")
		assert(t.f == "g")

		f[nil] = "h"
		assert(f[nil] == "h")
		assert(t.f == "h")

		assert(not pcall(function()
			local a = {}
			a[r] = 3
			return a
		end))
	end

	collectgarbage()

	assert(tostring(r) == tostring(s))
	assert(r[nil] == "x")
	assert(s[nil] == "x")
	-- assert(f[nil] == "h")

	assert(r[nil] == "x")
end

print "OK"
