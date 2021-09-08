#
#  CMakeLists.txt
#  lockfree_container
#
#  Created by Dmitrii Torkhov <dmitriitorkhov@gmail.com> on 07.09.2021.
#  Copyright © 2021 Dmitrii Torkhov. All rights reserved.
#

project(tests)

#

function(add name)
    add_executable(${name} ${name}.cpp)

    target_link_libraries(${name} oo::lockfree_container)

    set_target_properties(${name} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

    add_test(NAME ${name} COMMAND ${name})
endfunction()

#

add("test_multithreading")
add("test_blocking_wait")