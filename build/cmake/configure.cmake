if (USE_CPM)
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/scripts)
  include(CPM)
endif()
