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
		local t = { 0xa, 0xb, 0xc, 0xd }
		r = @t[2]
		assert(type(r) == "pointer")
		assert(tostring(r) == string.format("pointer: %p", r))
		assert(r[] == 0xb)

		r[] = 0xd
		assert(r[] == 0xd)
		assert(r[nil] == 0xd)
		assert(t[2] == 0xd)

		t[8] = 0xe
		assert(t[8] == 0xe)
		assert(r[] == 0xd)
		s = ptr.addr(t, 2) -- @t[2]
		assert(r == s)
		assert(t[5] == nil)
		assert(t[6] == nil)
		assert(t[7] == nil)

		t[2] = 0xff
		assert(t[2] == 0xff)
		assert(r[] == 0xff)

		t[3] = nil
		t[4] = nil
		for i = 1,10 do
			t["f".. i] = i
		end
		assert(t[2] == 0xff)
		assert(t[8] == 0xe)
		assert(r[] == 0xff)
		s = @t[2]
		assert(r == s)

		assert(not pcall(rawset, t, r, 3))
		assert(not pcall(ptr.addr, t, "f"))

		assert(not pcall(function() return @t["a"] end))
		t = nil
		assert(not pcall(function() return @t[1] end))
	end
	refs[1] = r
	refs[2] = s
	print "SCOPE"

	local box = (function()
		local t = { { "box of box" } }
		return @t[1]
	end)()

	print "BOX"
	-- once for table, once for box
	for _ = 1, 3 do
		collectgarbage()

		assert(tostring(r) == tostring(s))
		assert(r == refs[1])
		assert(r == refs[2])
		assert(r[] == 0xff)
		assert(s[] == 0xff)
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
