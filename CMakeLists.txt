add_library(ccs811
	ccs811.c
)

target_include_directories(ccs811 PUBLIC
	.
)

target_link_libraries(ccs811 pico_stdlib hardware_i2c)