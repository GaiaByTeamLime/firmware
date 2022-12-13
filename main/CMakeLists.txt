idf_component_register(SRCS 
                            "blufi_main.c"
                            "blufi_security.c"
                            "blufi_init.c"
                            "gaia.c" 
                            "main.c"
                    INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
