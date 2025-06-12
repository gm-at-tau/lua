print("testing pointer")
local gc = { "incremental", "generational" }

for _, opt in ipairs(gc) do
	collectgarbage()
	print(opt)
	collectgarbage(opt)
	collectgarbage()

	local refs = {}
	local r, s
	do
		local t = { "a", "b", "c" }
		r = ptr.addr(t, 2)
		assert(type(r) == "pointer")
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
		ptr.addr(t, 1) -- unused
	end
	refs[1] = r
	refs[2] = s
	print "SCOPE"

	local box = (function()
		local t = { { "box of box" } }
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

	assert(type(box[nil]) == "table")
	assert(box[nil][1] == "box of box")
end

collectgarbage()

do
	print "TM"
	local tm = {}
	function tm.__addr(t, i)
		return ptr.rawaddr(t, i + 1)
	end

	local t = setmetatable({ "x", "y", "z" }, tm)
	local r = ptr.addr(t, 1)
	assert(r[nil] == "y")
	collectgarbage()
end

collectgarbage()

do
	local t = { 0 }
	local r = ptr.addr(t, 1)
	t[1] = r
	collectgarbage()
end

collectgarbage()


print "OK"
