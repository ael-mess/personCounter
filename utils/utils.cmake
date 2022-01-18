file(GLOB SRC_FILES LIST_DIRECTORIES false
    ${PROJECT_SOURCE_DIR}/main/*.c
    ${PROJECT_SOURCE_DIR}/main/*.cpp
    ${PROJECT_SOURCE_DIR}/main/include/*.h
    )

file(GLOB CFG_FILES LIST_DIRECTORIES false
    ${PROJECT_SOURCE_DIR}/main/CMakeLists.txt
    ${PROJECT_SOURCE_DIR}/main/component.mk
    ${PROJECT_SOURCE_DIR}/main/Kconfig.projbuild
    ${PROJECT_SOURCE_DIR}/utils/*
    ${PROJECT_SOURCE_DIR}/*
    )

# Format configuration
add_custom_target(format
    COMMENT "Starting clang-fromat"
    DEPENDS ${SRC_FILES}
    COMMAND clang-format --version
    COMMAND ls ${PROJECT_SOURCE_DIR}/.clang-format > /dev/null
    COMMAND clang-format --verbose -i -style=file ${SRC_FILES}
    VERBATIM
    )

# Format test configuration
add_custom_target(formatcheck
    COMMENT "Starting clang-format"
    DEPENDS ${SRC_FILES}
    COMMAND clang-format --version
    COMMAND ls ${PROJECT_SOURCE_DIR}/.clang-format > /dev/null
    COMMAND ${PROJECT_SOURCE_DIR}/utils/formatcheck.sh ${PROJECT_SOURCE_DIR}/.clang-format ${SRC_FILES}
    VERBATIM
    )

# TODOs grep
add_custom_target(todocheck
    COMMENT "Launching grep"
    DEPENDS ${SRC_FILES}
    COMMAND grep -v "grep" ${SRC_FILES} ${CFG_FILES} | grep TODO
    VERBATIM
    )

# Static analyzer configuration
add_custom_target(cppcheck
    COMMENT "Starting cppcheck"
    DEPENDS ${SRC_FILES}
    COMMAND cppcheck --version
    COMMAND cppcheck --max-configs=512 --quiet --enable=all --suppress=unusedFunction --suppress=missingInclude --error-exitcode=1 ${SRC_FILES}
    VERBATIM
    )
