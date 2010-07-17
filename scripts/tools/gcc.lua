local env = ...

-- load the generic C toolset first
load_toolset("generic-cpp", env)

local native = require("tundra.native")

env:set_many {
	["HEADERS_EXTS"] = { ".h", ".hpp", ".hh", ".hxx" },
	["NATIVE_SUFFIXES"] = { ".c", ".cpp", ".cc", ".cxx", ".a", ".o" },
	["OBJECTSUFFIX"] = ".o",
	["PROGSUFFIX"] = "",
	["LIBSUFFIX"] = ".a",
	["CC"] = "gcc",
	["C++"] = "g++",
	["LIB"] = "ar",
	["LD"] = "gcc",
	["CPPDEFS"] = "",
	["CCOPTS"] = "-Wall",
	["CCCOM"] = "$(CC) -c $(CPPDEFS:p-D) $(CPPPATH:f:p-I) $(CCOPTS) -o $(@) $(<)",
	["CXXCOM"] = "$(CCCOM)",
	["PROGLIBS"] = "",
	["PROGOPTS"] = "",
	["PROGCOM"] = "$(LD) $(PROGOPTS) $(PROGLIBS) -o $(@) $(<)",
	["LIBOPTS"] = "",
	["LIBCOM"] = "$(LIB) -r $(LIBOPTS) $(@) $(<)",
}
