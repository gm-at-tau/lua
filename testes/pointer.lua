print("testing pointer")
local gc = { "incremental", "generational" }

for _, opt in ipairs(gc) do
	collectgarbage()
	print(opt)
	collectgarbage(opt)
	collectgarbage()

	local refs = {}
	local a0, a1
	do
		local t = { 0xa, 0xb, 0xc, 0xd, a = 0xa }
		a0 = @t[2]
		assert(type(a0) == "pointer")
		assert(tostring(a0) == string.format("pointer: %p", a0))
		assert(a0[] == 0xb)

		a0[] = 0xd
		assert(a0[] == 0xd)
		assert(a0[nil] == 0xd)
		assert(t[2] == 0xd)

		t[8] = 0xe
		assert(t[8] == 0xe)
		assert(a0[] == 0xd)
		a1 = ptr.addr(t, 2) -- @t[2]
		assert(a0 == a1)
		assert(t[5] == nil)
		assert(t[6] == nil)
		assert(t[7] == nil)

		t[2] = 0xff
		assert(t[2] == 0xff)
		assert(a0[] == 0xff)

		t[3] = nil
		t[4] = nil
		for i = 1,10 do
			t["f".. i] = i
		end
		assert(t[2] == 0xff)
		assert(t[8] == 0xe)
		assert(a0[] == 0xff)
		a1 = @t[2]
		assert(a0 == a1)

		-- assert(not pcall(rawset, t, r, 3))

		local h0, h1
		h0 = @t["a"]
		assert(type(h0) == "pointer")
		assert(tostring(h0) == string.format("pointer: %p", h0))
		assert(h0[] == 0xa)

		h0[] = 0x1a
		assert(h0[] == 0x1a)
		assert(h0[nil] == 0x1a)
		assert(t.a == 0x1a)

		t.b = 0x1b
		t.c = 0x1c
		t.d = 0x1d
		t.e = 0x1e
		assert(t.e == 0x1e)
		assert(h0[] == 0x1a)
		h1 = ptr.addr(t, "a") -- @t["a"]
		assert(h0 == h1)

		t.a = 0xa
		assert(t.a == 0xa)
		assert(h0[] == 0xa)

		-- assert(not pcall(ptr.addr, t, "f"))

		-- assert(not pcall(function() return @t["a"] end))
		t = nil
		assert(not pcall(function() return @t[1] end))
	end
	refs[1] = a0
	refs[2] = a1
	print "SCOPE"

	local box = (function()
		local t = { { "box of box" } }
		return @t[1]
	end)()

	print "BOX"
	-- once for table, once for box
	for _ = 1, 3 do
		collectgarbage()

		assert(tostring(a0) == tostring(a1))
		assert(a0 == refs[1])
		assert(a0 == refs[2])
		assert(a0[] == 0xff)
		assert(a1[] == 0xff)
	end

	assert(type(box[]) == "table")
	assert(box[][1] == "box of box")
end

collectgarbage()

do
	print "TM"
	local tm = {}
	function tm.__addr(t, i)
		return ptr.rawaddr(t, i + 1)
	end

	local t = setmetatable({ 0xa, 0xb, 0xc }, tm)
	local r = @t[1]
	assert(r[] == 0xb)
	collectgarbage()
end

collectgarbage()

do
	local t = { 0 }
	local r = @t[1]
	t[1] = r
	collectgarbage()
end

collectgarbage()


print "OK"
