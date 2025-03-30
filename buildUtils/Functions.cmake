####################################################################################################
function(add_sources_recursively target_name source_dir)
    if (NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist. Please create the target before calling this function.")
    endif ()
    file(GLOB_RECURSE sources "${source_dir}/*.cpp" "${source_dir}/*.c" "${source_dir}/*.h" "${source_dir}/*.hpp")
    if (sources)
        target_sources(${target_name} PRIVATE ${sources})
    else ()
        message(WARNING "No source files found in directory: ${source_dir}")
    endif ()
endfunction()
####################################################################################################
function(search_text_files dir search_strings)
    # Record start time
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
        string(TIMESTAMP start_time "%s")
    else()
        message(FATAL_ERROR "CMake 3.17 or higher is required for timing support.")
    endif()

    file(GLOB_RECURSE cpp_files "${dir}/*.cpp")
    file(GLOB_RECURSE h_files "${dir}/*.h")

    # Combine file lists
    set(source_files ${cpp_files} ${h_files})
    list(LENGTH source_files file_count)
    message(STATUS "Found ${file_count} files to search.")

    # Handle search strings list
    if(NOT "${search_strings}" STREQUAL "")
        # Escape list separators within items
        string(REPLACE ";" "\\;" search_strings "${search_strings}")
        # Now convert to a proper CMake list
        string(REPLACE "|" ";" search_strings "${search_strings}")
    endif()

    message(STATUS "Searching for: ${search_strings}")

    foreach(file IN LISTS source_files)
        file(READ "${file}" file_content)

        set(found_strings "")
        foreach(search_string IN LISTS search_strings)
            string(FIND "${file_content}" "${search_string}" found_pos)
            if(NOT ${found_pos} EQUAL -1)
                list(APPEND found_strings "${search_string}")
            endif()
        endforeach()

        list(LENGTH found_strings match_count)
        if(match_count GREATER 0)
            list(JOIN found_strings ", " found_strings_formatted)
            message(STATUS "Found in ${file}: ${found_strings_formatted}")
        endif()
    endforeach()

    # Record end time and calculate duration
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.17")
        string(TIMESTAMP end_time "%s")
        math(EXPR elapsed_time "${end_time} - ${start_time}")
        message(STATUS "Search completed in ${elapsed_time} seconds.")
    endif()
endfunction()
####################################################################################################
