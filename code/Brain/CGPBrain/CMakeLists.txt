option(enable_Brain_CGP "Enable Module CGPBrain" ON)
if (enable_Brain_CGP)
  register_module(Brain CGP)
  target_sources(${EXE} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/CGPBrain.cpp)
  target_sources(${EXE} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/CGPBrain.h)
endif()
