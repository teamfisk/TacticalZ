# Xerces_FOUND
# Xerces_INCLUDE_DIRS
# Xerces_LIBRARIES

find_path(Xerces_INCLUDE_DIR xercesc/dom/dom.hpp
	/usr/local/include
	/usr/include
)

find_library(Xerces_LIBRARY
	NAMES
	xerces-c_3
	xerces-c_3D
	PATHS
	/usr/local/lib
	/usr/lib
)

set(Xerces_INCLUDE_DIRS ${Xerces_INCLUDE_DIR})
set(Xerces_LIBRARIES ${Xerces_LIBRARY})

find_package_handle_standard_args(Xerces DEFAULT_MSG Xerces_LIBRARY Xerces_INCLUDE_DIR)
mark_as_advanced(Xerces_FOUND Xerces_INCLUDE_DIR Xerces_LIBRARY)
