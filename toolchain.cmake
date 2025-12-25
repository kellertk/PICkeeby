set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR PIC16)

# XC8 compiler path
set(XC8_PATH "/opt/microchip/xc8/v3.10" CACHE PATH "Path to XC8 compiler")

# Compiler executables
set(CMAKE_C_COMPILER "${XC8_PATH}/bin/xc8-cc")
set(CMAKE_ASM_COMPILER "${XC8_PATH}/bin/xc8-cc")
set(CMAKE_AR "${XC8_PATH}/bin/xc8-ar")
set(CMAKE_RANLIB "")

# Tell CMake the compiler works (skip test compilation)
set(CMAKE_C_COMPILER_WORKS YES)
set(CMAKE_ASM_COMPILER_WORKS YES)

# Don't detect ABI (causes issues with XC8)
set(CMAKE_C_ABI_COMPILED YES)
set(CMAKE_ASM_ABI_COMPILED YES)

# Output extensions - XC8 uses .p1 for intermediate files
set(CMAKE_C_OUTPUT_EXTENSION .p1)
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_ASM_OUTPUT_EXTENSION .p1)
set(CMAKE_ASM_OUTPUT_EXTENSION_REPLACE 1)

# Override link command to use .p1 files
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Archiver command
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> <LINK_FLAGS> <TARGET> <OBJECTS>")
set(CMAKE_ASM_CREATE_STATIC_LIBRARY "<CMAKE_AR> <LINK_FLAGS> <TARGET> <OBJECTS>")

# Dependency file patterns
set(CMAKE_DEPFILE_FLAGS_C "-MD -MF <DEP_FILE>")
set(CMAKE_DEPFILE_FLAGS_ASM "-MD -MF <DEP_FILE>")
