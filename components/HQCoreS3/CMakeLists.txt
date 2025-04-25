set(src_dirs        
  ./source
  )

set(include_dirs    
  ./include
  )

set(requires 
  driver
  esp_timer
  esp_lcd_touch_cst816s
  lvgl
  esp32-camera
  fatfs
  mpu6050
  )

idf_component_register(
  SRC_DIRS ${src_dirs} 
  INCLUDE_DIRS ${include_dirs} 
  REQUIRES ${requires}
  )