
set(cr_common_SRCS
  assert.h
  assert_builder.h
  assert_builder.cpp
  assert_error.h
  error.h
  error.cpp
  exception.h
  exception.cpp
  scope_guard.h
  throw.h
  )
  
add_library(cr_common ${cr_common_SRCS})
install(TARGETS cr_common DESTINATION lib)