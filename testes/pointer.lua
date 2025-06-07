print("testing pointer")
local gc = { "incremental", "generational" }

for _, opt in ipairs(gc) do
	print(opt)
	collectgarbage(opt)

	local refs = {}
	local r, s
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

		assert(not pcall(rawset, t, r, 3))
		assert(not pcall(ptr.addr, t, "f"))

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
	for _ = 1, 3 do
		collectgarbage()

		assert(tostring(r) == tostring(s))
		assert(r == refs[1])
		assert(r == refs[2])
		assert(r[nil] == "x")
		assert(s[nil] == "x")
	end

	assert(box[nil] == 42)
end

print "OK"
