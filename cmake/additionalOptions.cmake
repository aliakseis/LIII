set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 14)

if(WIN32)

set(ADDITIONAL_CL_OPTIMIZATION_OPTIONS
	/Gy # function level linking
	/GF # string pooling
	/GL # whole program optimization
	/Oi # intrinsic functions
	/Ot # fast code
	/Ob2 # inline expansion
	/Ox  # full optimization
	/fp:except- /fp:fast
)

set(ADDITIONAL_CL_OPTIMIZATION_OPTIONS_projectName
	/Os  # small code
	/Og  # Turn on loop, common subexpression and register optimizations
	/Ob0 # do not inline
	/O2  # maximize speed
	/fp:precise
)

set(ADDITIONAL_LINKER_OPTIMIZATION_OPTIONS
	/INCREMENTAL:NO
	/LTCG
	/DEBUG
)

endif(WIN32)