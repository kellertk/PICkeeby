cmake_minimum_required(VERSION 3.24.0)

# Set toolchain file before project()
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/toolchain.cmake")

project(keeby LANGUAGES C ASM)

# Configuration options
set(DEVICE "PIC16F716" CACHE STRING "Target PIC device")
set(XC8_PATH "/opt/microchip/xc8/v3.10" CACHE PATH "Path to XC8 compiler installation")
set(DFP_PATH "$ENV{HOME}/.mchp_packs/Microchip/PIC16Fxxx_DFP/1.7.162" CACHE PATH "Path to Device Family Pack")

# Compiler flags common to both compile and link stages
set(COMMON_FLAGS
    -mcpu=${DEVICE}
    -mdfp=${DFP_PATH}/xc8
    -fno-short-double
    -fno-short-float
    -O2
    -maddrqual=ignore
    -mwarn=-3
    -msummary=-psect,-class,+mem,-hex,+file
    -ginhx32
    -Wl,--data-init
    -mno-keep-startup
    -mno-osccal
    -mno-resetbits
    -mno-save-resetbits
    -mdownload
    -mstackcall
    -mno-default-config-bits
    -std=c99
    -gdwarf-3
    -frors
    -finterrupt-context-loops
)

# Source files
set(SOURCES
    keymap.c
    main.c
    ps2_send.c
)

# Compile definitions
set(COMPILE_DEFS
    -D__${DEVICE}__
    -DXPRJ_default=default
)

# Build object files using custom commands
set(OBJECTS "")
foreach(SRC ${SOURCES})
    get_filename_component(SRC_NAME ${SRC} NAME_WE)
    set(OBJ_FILE "${CMAKE_BINARY_DIR}/${SRC_NAME}.p1")
    add_custom_command(
        OUTPUT ${OBJ_FILE}
        COMMAND ${CMAKE_C_COMPILER} ${COMPILE_DEFS} -c ${COMMON_FLAGS} -o ${OBJ_FILE} ${CMAKE_SOURCE_DIR}/${SRC}
        DEPENDS ${CMAKE_SOURCE_DIR}/${SRC}
        COMMENT "Compiling ${SRC}"
        VERBATIM
    )
    list(APPEND OBJECTS ${OBJ_FILE})
endforeach()

# Link executable
set(OUTPUT_FILE "${CMAKE_SOURCE_DIR}/out/keeby.elf")
add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/out
    COMMAND ${CMAKE_C_COMPILER}
        -Wl,-Map=${CMAKE_BINARY_DIR}/mem.map
        -Wl,--defsym=__MPLAB_BUILD=1
        ${COMMON_FLAGS}
        -Wl,--memorysummary,${CMAKE_BINARY_DIR}/memoryfile.xml
        ${OBJECTS}
        -o ${OUTPUT_FILE}
    DEPENDS ${OBJECTS}
    COMMENT "Linking keeby.elf"
    VERBATIM
)

# Create a custom target
add_custom_target(${PROJECT_NAME} ALL DEPENDS ${OUTPUT_FILE})
