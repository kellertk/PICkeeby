# cmake files support debug production
include("${CMAKE_CURRENT_LIST_DIR}/rule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/file.cmake")

set(keeby_default_library_list )

# Handle files with suffix (s|as|asm|AS|ASM|As|aS|Asm), for group default-XC8
if(keeby_default_default_XC8_FILE_TYPE_assemble)
add_library(keeby_default_default_XC8_assemble OBJECT ${keeby_default_default_XC8_FILE_TYPE_assemble})
    keeby_default_default_XC8_assemble_rule(keeby_default_default_XC8_assemble)
    list(APPEND keeby_default_library_list "$<TARGET_OBJECTS:keeby_default_default_XC8_assemble>")

endif()

# Handle files with suffix S, for group default-XC8
if(keeby_default_default_XC8_FILE_TYPE_assemblePreprocess)
add_library(keeby_default_default_XC8_assemblePreprocess OBJECT ${keeby_default_default_XC8_FILE_TYPE_assemblePreprocess})
    keeby_default_default_XC8_assemblePreprocess_rule(keeby_default_default_XC8_assemblePreprocess)
    list(APPEND keeby_default_library_list "$<TARGET_OBJECTS:keeby_default_default_XC8_assemblePreprocess>")

endif()

# Handle files with suffix [cC], for group default-XC8
if(keeby_default_default_XC8_FILE_TYPE_compile)
add_library(keeby_default_default_XC8_compile OBJECT ${keeby_default_default_XC8_FILE_TYPE_compile})
    keeby_default_default_XC8_compile_rule(keeby_default_default_XC8_compile)
    list(APPEND keeby_default_library_list "$<TARGET_OBJECTS:keeby_default_default_XC8_compile>")

endif()


# Main target for this project
add_executable(keeby_default_image_90VDpUxK ${keeby_default_library_list})

set_target_properties(keeby_default_image_90VDpUxK PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${keeby_default_output_dir})
set_target_properties(keeby_default_image_90VDpUxK PROPERTIES OUTPUT_NAME "default")
set_target_properties(keeby_default_image_90VDpUxK PROPERTIES SUFFIX ".elf")

target_link_libraries(keeby_default_image_90VDpUxK PRIVATE ${keeby_default_default_XC8_FILE_TYPE_link})


# Add the link options from the rule file.
keeby_default_link_rule(keeby_default_image_90VDpUxK)



