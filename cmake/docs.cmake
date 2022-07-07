# SPDX-License-Identifier: MIT Copyright (c) 2022 Jai Bellare See
# <https://opensource.org/licenses/MIT/> or LICENSE.md Project homepage:
# <https://github.com/strangeQuark1041/samarium>

# https://vicrucann.github.io/tutorials/quick-cmake-doxygen/

# https://gist.github.com/strangeQuark1041/acba76d25b213327a568a9f3c9152550

if(CMAKE_BUILD_TYPE MATCHES "^[Rr]elease" AND BUILD_DOCS)
    # check if Doxygen is installed
    find_program(DOXYGEN_EXECUTABLE NAMES doxygen)

    if(DOXYGEN_EXECUTABLE)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(
            docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/src/Doxyfile
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT
                "Generating API documentation with Doxygen. Open build/docs/index.html"
            VERBATIM
        )
    else()
        message(FATAL_ERROR "Doxygen need to be installed to generate documentation")
    endif()
endif()
