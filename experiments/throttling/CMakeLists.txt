# Executáveis: somente os .cc na raiz deste experimento.
file(GLOB experiment_sources CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

# Código compartilhado dos modelos
file(GLOB model_sources CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/models/*.cc")

foreach(source_file ${experiment_sources})

  get_filename_component(exec_name ${source_file} NAME_WLE)

  build_exec(
    EXECNAME ${exec_name}
    EXECNAME_PREFIX "scratch_experiments_throttling_"
    SOURCE_FILES
      ${source_file}
      ${model_sources}
    LIBRARIES_TO_LINK
      "${ns3-libs}"
      "${ns3-contrib-libs}"
    EXECUTABLE_DIRECTORY_PATH
      "${CMAKE_OUTPUT_DIRECTORY}/scratch/experiments/throttling"
  )

endforeach()
