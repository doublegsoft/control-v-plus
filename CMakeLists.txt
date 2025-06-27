#                  _             _                 
#   ___ ___  _ __ | |_ _ __ ___ | |    __   __ _   
#  / __/ _ \| '_ \| __| '__/ _ \| |____\ \ / /| |_ 
# | (_| (_) | | | | |_| | | (_) | |_____\ V /_   _|
#  \___\___/|_| |_|\__|_|  \___/|_|      \_/  |_|  
#

cmake_minimum_required(VERSION 3.13) 

project(control-v-plus                 
  VERSION 1.0.0                    
  DESCRIPTION "a project for collaborative tool across networks to make development productive" 
  LANGUAGES C CXX                
)

# Set C++ standard (common options: 98, 11, 14, 17, 20, 23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING "Build the testing tree" OFF)

include_directories(
  /usr/include/gtk-3.0
  /usr/include/glib-2.0
  /usr/include/libayatana-appindicator3-0.1
  /usr/lib/x86_64-linux-gnu/glib-2.0/include
  /usr/include/pango-1.0
  /usr/include/harfbuzz 
  /usr/include/freetype2 
  /usr/include/libpng16 
  /usr/include/libmount 
  /usr/include/blkid 
  /usr/include/fribidi 
  /usr/include/cairo 
  /usr/include/pixman-1 
  /usr/include/gdk-pixbuf-2.0 
  /usr/include/x86_64-linux-gnu 
  /usr/include/webp 
  /usr/include/gio-unix-2.0 
  /usr/include/atk-1.0 
  /usr/include/at-spi2-atk/2.0 
  /usr/include/at-spi-2.0 
  /usr/include/dbus-1.0 
  /usr/lib/x86_64-linux-gnu/dbus-1.0/include 
  /usr/include/libayatana-appindicator3-0.1 
  /usr/include/libayatana-indicator3-0.4 
  /usr/include/libayatana-ido3-0.4 
  /usr/include/libdbusmenu-glib-0.4
)

# Add executable
add_executable(${PROJECT_NAME} src/main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  ayatana-appindicator3 
  ayatana-indicator3 
  ayatana-ido3-0.4 
  dbusmenu-glib 
  gtk-3 
  gdk-3 
  z 
  pangocairo-1.0 
  pango-1.0 
  harfbuzz 
  atk-1.0 
  cairo-gobject 
  cairo 
  gdk_pixbuf-2.0 
  gio-2.0 
  gobject-2.0 
  glib-2.0 
)

