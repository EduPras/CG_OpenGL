project(LearnOpenGl)
message("Inside external")
add_library(
    external
    glad/glad.c
    glad/glad.h
    KHR/khrplatform.h
)
