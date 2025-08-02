disable_rtti()

file(GLOB_RECURSE SOURCES *.cpp *.h)

new_library_shared(mod_discord "${SOURCES}" "")

target_include_directories(mod_discord PRIVATE "../dpp/include/dpp")
#target_link_directories(mod_discord PRIVATE "../dpp/lib/dpp")

if(WIN32)
target_link_libraries(mod_discord libdpp)
else()
target_link_libraries(mod_discord dpp)
endif()
set_target_properties(mod_discord PROPERTIES FOLDER modules)

target_compile_features(mod_discord PUBLIC cxx_std_17)
