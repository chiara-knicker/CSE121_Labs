idf_component_register(SRCS "Lab4_1.c"
                            "DFRobot_LCD.c"
			    "i2c_api.c"
                    INCLUDE_DIRS ".")

# target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)
