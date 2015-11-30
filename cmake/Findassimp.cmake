find_path(
	assimp_INCLUDE_DIR
	NAMES 
	assimp/postprocess.h 
	assimp/scene.h 
	assimp/version.h 
	assimp/config.h 
	assimp/cimport.h
	PATHS /usr/local/include/
)

find_library(
	assimp_LIBRARY
	NAMES assimp
	PATHS /usr/local/lib/
)

set(assimp_INCLUDE_DIRS ${assimp_INCLUDE_DIR})
set(assimp_LIBRARIES ${assimp_LIBRARY})

find_package_handle_standard_args(assimp REQUIRED_VARS assimp_INCLUDE_DIR assimp_LIBRARY)
mark_as_advanced(assimp_INCLUDE_DIR assimp_LIBRARY)