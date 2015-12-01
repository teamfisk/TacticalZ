# XERCES_FOUND
# XERCES_INCLUDE_DIRS
# XERCES_LIBRARIES

find_path(XERCES_INCLUDE_DIR xercesc/dom/dom.hpp
	/usr/local/include
	/usr/include
)

find_library(XERCES_LIBRARY
	NAMES
	xerces-c_3
	xerces-c_3D
	PATHS
	/usr/local/lib
	/usr/lib
)

set(XERCES_INCLUDE_DIRS ${XERCES_INCLUDE_DIR})
set(XERCES_LIBRARIES ${XERCES_LIBRARY})

find_package_handle_standard_args(Xerces DEFAULT_MSG XERCES_LIBRARY XERCES_INCLUDE_DIR)
mark_as_advanced(Xerces_FOUND XERCES_INCLUDE_DIR XERCES_LIBRARY)