-- Build file for testing window input handling
--
-- (c)2012 Travis Sanchez
local homelib = "../../lib"
local homeinc = "../../"
local homebin = "../../bin"
local externlib = homeinc.."/EXTERN_LIB"

-- DEPS

-- Solution "FISHY"
solution "FISHY"
   -- Build target: protoc
   project "protoc"
      kind "ConsoleApp"
      language "C++"
      targetdir (homebin)

      libdirs { (homelib) }
      includedirs { (homeinc), (externlib) }

      files { "**.h", "**.hpp", "**.cpp", "**.cc", "**.inl" }
      files {
        "../../CORE/types.h",
        "../../CORE/ARCH/mutex.h",
        "../../CORE/ARCH/process.h",
        "../../CORE/ARCH/process.cpp",
        "../../CORE/ARCH/timer.h",
        "../../CORE/ARCH/timer.cpp",
        "../../CORE/BASE/config.h",
        "../../CORE/BASE/config.inl",
        "../../CORE/BASE/config.cpp",
        "../../CORE/BASE/logging.h",
        "../../CORE/BASE/logging.cpp",
        "../../CORE/HASH/crc32.h",
        "../../CORE/HASH/crc32.cpp",
        "../../CORE/UTIL/lexical_cast.h",
        "../../CORE/UTIL/regex.h",
        "../../CORE/UTIL/regex.cpp",
        "../../CORE/UTIL/stringutil.h",
        "../../CORE/UTIL/stringutil.cpp",
        "../../CORE/UTIL/tokenizer.h",
        "../../CORE/UTIL/tokenizer.inl"
      }

      -- Config: Debug
      configuration "Debug"
         targetsuffix "d"
         defines { "DEBUG", "_CRT_SECURE_NO_WARNINGS" }
         flags { "Symbols" }
 
      -- Config: Release
      configuration "Release"
         defines { "NDEBUG", "_CRT_SECURE_NO_WARNINGS" }
         flags { "Optimize" }

   project "protoclib"
      kind "StaticLib"
      language "C++"
      targetdir (homebin)

      libdirs { (homelib) }
      includedirs { (homeinc), (externlib) }

      files { "**.h", "**.hpp", "**.cpp", "**.cc", "**.inl" }
      files {
        "../../CORE/types.h",
        "../../CORE/ARCH/mutex.h",
        "../../CORE/ARCH/process.h",
        "../../CORE/ARCH/process.cpp",
        "../../CORE/ARCH/timer.h",
        "../../CORE/ARCH/timer.cpp",
        "../../CORE/BASE/config.h",
        "../../CORE/BASE/config.inl",
        "../../CORE/BASE/config.cpp",
        "../../CORE/BASE/logging.h",
        "../../CORE/BASE/logging.cpp",
        "../../CORE/HASH/crc32.h",
        "../../CORE/HASH/crc32.cpp",
        "../../CORE/UTIL/lexical_cast.h",
        "../../CORE/UTIL/regex.h",
        "../../CORE/UTIL/regex.cpp",
        "../../CORE/UTIL/stringutil.h",
        "../../CORE/UTIL/stringutil.cpp",
        "../../CORE/UTIL/tokenizer.h",
        "../../CORE/UTIL/tokenizer.inl"
      }
	  excludes { "main.cpp" }

      -- Config: Debug
      configuration "Debug"
         targetsuffix "d"
         defines { "DEBUG", "_CRT_SECURE_NO_WARNINGS" }
         flags { "Symbols" }
 
      -- Config: Release
      configuration "Release"
         defines { "NDEBUG", "_CRT_SECURE_NO_WARNINGS" }
         flags { "Optimize" }
