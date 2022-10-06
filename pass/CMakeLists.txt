# add_library(SkeletonPass MODULE
#     # List your source files here.
#     Skeleton.cpp
    
# )

# # Use C++11 to compile our pass (i.e., supply -std=c++11).
# target_compile_features(SkeletonPass PRIVATE cxx_range_for cxx_auto_type)


# # Get proper shared-library behavior (where symbols are not necessarily
# # resolved when the shared library is linked) on OS X.
# if(APPLE)
#     set_target_properties(SkeletonPass PROPERTIES
#         LINK_FLAGS "-undefined dynamic_lookup"
#     )
# endif(APPLE)

set(INCLUDE_DIR ../include)
set(PASS_SOURCES 
    ResearchPasses.cpp
    SkeletonPass.cpp
    MemoryPass.cpp
    MutatePass.cpp
    LabelPass.cpp
    RegisterInfoPass.cpp
    RegisterExitPass.cpp
    WindowPass.cpp
    SingleWindowPass.cpp
    FuncNameChangePass.cpp
)

add_library(ResearchPasses SHARED ${PASS_SOURCES})
target_include_directories(ResearchPasses PRIVATE ${INCLUDE_DIR})
