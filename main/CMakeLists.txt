idf_component_register(
  SRCS "app_main.cpp"
  PRIV_REQUIRES spi_flash 
  INCLUDE_DIRS ""
  REQUIRES HQCoreS3 nvs_flash
  )