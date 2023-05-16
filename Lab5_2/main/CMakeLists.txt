idf_component_register(SRCS "station_example_main.c"
			    "i2c_api.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES server_root_cert.pem local_server_cert.pem)
