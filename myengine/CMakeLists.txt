project(myengine)

set(myengine_headers_public
    logging.h
    )
set(myengine_headers_private)
source_group("Header Files\\Public"  ${myengine_headers_public})
source_group("Header Files\\Private" ${myengine_headers_private})
install(
  FILES ${myengine_headers_public}
  DESTINATION include/myegine
)

# No source files just yet.
#add_library(
#  myegine
#  ${myengine_headers_public}
#)
