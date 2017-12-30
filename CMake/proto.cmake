function(proto_generate SOURCES)
  if(NOT ARGN)
    message(SEND_ERROR "proto_generate() called without schema files")
  endif()

  set(${SOURCES})
  foreach(schema_file ${ARGN})
    get_filename_component(file_path "${schema_file}" ABSOLUTE)
    get_filename_component(file_dir "${file_path}" PATH)
    file(RELATIVE_PATH genfile_rel_dir ${FISHY_SOURCE_DIR} ${file_path})
    set(output_base "${FISHY_GENERATED_ROOT}/${genfile_rel_dir}")
    string(REPLACE ".proto" ".pb" output_base ${output_base})

    set(protoc_location $<TARGET_FILE:protoc>)
    # get_property(protoc_location TARGET protoc PROPERTY LOCATION)
    add_custom_command(
      OUTPUT  "${output_base}.cpp" "${output_base}.h"
      COMMAND "${protoc_location}"
      ARGS --infile "${file_path}"
           --outfileprefix "${output_base}"
      DEPENDS protoc
      WORKING_DIRECTORY ${FISHY_SOURCE_DIR}
      COMMENT "Generating proto for: ${schema_file}"
      USES_TERMINAL
    )

    list(APPEND ${SOURCES} "${output_base}.cpp")
    list(APPEND ${SOURCES} "${output_base}.h")
  endforeach()

  set_source_files_properties(${${SOURCES}} PROPERTIES GENERATED TRUE)
  set(${SOURCES} ${${SOURCES}} PARENT_SCOPE)
endfunction()
