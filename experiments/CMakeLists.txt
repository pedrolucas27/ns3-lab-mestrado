# Cada .cc com main() vira um executável independente.
# Sem este arquivo, o scratch do ns-3 trata a pasta inteira como um único
# programa e falha quando há mais de um main() (ex.: hello-simulator + simulator-1).

file(GLOB_RECURSE experiment_sources CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

foreach(source_file ${experiment_sources})
  get_filename_component(exec_name ${source_file} NAME_WLE)
  get_filename_component(source_dir ${source_file} DIRECTORY)
  file(RELATIVE_PATH rel_dir "${CMAKE_CURRENT_SOURCE_DIR}" "${source_dir}")

  if(rel_dir STREQUAL "")
    set(exec_prefix "scratch_experiments_")
    set(out_dir "${CMAKE_OUTPUT_DIRECTORY}/scratch/experiments")
  else()
    string(REPLACE "/" "_" rel_prefix "${rel_dir}")
    set(exec_prefix "scratch_experiments_${rel_prefix}_")
    set(out_dir "${CMAKE_OUTPUT_DIRECTORY}/scratch/experiments/${rel_dir}")
  endif()

  build_exec(
    EXECNAME ${exec_name}
    EXECNAME_PREFIX ${exec_prefix}
    SOURCE_FILES ${source_file}
    LIBRARIES_TO_LINK "${ns3-libs}" "${ns3-contrib-libs}"
    EXECUTABLE_DIRECTORY_PATH ${out_dir}
  )
endforeach()
