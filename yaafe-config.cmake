get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/yaafe-targets.cmake)
get_filename_component(yaafe_INCLUDE_DIRS "${SELF_DIR}/../../include" ABSOLUTE)
