# Subpastas com CMakeLists próprio (ex.: circuit-breaker).
# O GLOB do scratch do ns-3 não desce pelo symlink experiments/, então
# este arquivo precisa existir para o add_subdirectory(experiments) enxergar os alvos.
file(GLOB children RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" CONFIGURE_DEPENDS "*")
foreach(child ${children})
  if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${child}"
     AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt")
    add_subdirectory(${child})
  endif()
endforeach()
