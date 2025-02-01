cmake_minimum_required(VERSION 3.8)
project(deteksi_openvn)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(OpenCV REQUIRED)
find_package(rosidl_default_generators REQUIRED) # Jika Anda menggunakan message custom
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(OpenVINO REQUIRED)

# Export dependencies for downstream packages (important!)
ament_export_dependencies(rclcpp OpenCV sensor_msgs cv_bridge image_transport OpenVINO)
ament_export_include_directories(include) # Jika Anda punya header di direktori include

add_executable(get_frm src/ambil_frame.cpp)
add_executable(get_frm_2 src/ambil_frame2.cpp)
add_executable(prosses_frm src/proses_deteksi.cpp)
add_executable(prosses_frm_2 src/proses_deteksi2.cpp)

# Use ament_target_dependencies for ROS 2 dependencies
ament_target_dependencies(get_frm rclcpp  sensor_msgs cv_bridge image_transport)
ament_target_dependencies(get_frm_2 rclcpp OpenCV sensor_msgs cv_bridge image_transport)

ament_target_dependencies(prosses_frm rclcpp OpenCV sensor_msgs cv_bridge image_transport OpenVINO)
ament_target_dependencies(prosses_frm_2 rclcpp OpenCV sensor_msgs cv_bridge image_transport OpenVINO)


# Target-specific include directories (best practice)
target_include_directories(get_frm PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>) # Include dirs project
target_include_directories(get_frm_2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>) # Include dirs project

target_include_directories(prosses_frm PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>) # Include dirs project
target_include_directories(prosses_frm_2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>) # Include dirs project

# Link libraries (OpenCV and OpenVINO)
target_link_libraries(get_frm ${OpenCV_LIBRARIES})
target_link_libraries(get_frm_2 ${OpenCV_LIBRARIES})

target_link_libraries(prosses_frm ${OpenCV_LIBRARIES} openvino::runtime)
target_link_libraries(prosses_frm_2 ${OpenCV_LIBRARIES} openvino::runtime)


install(TARGETS
  get_frm
  get_frm_2
  prosses_frm
  prosses_frm_2
  DESTINATION lib/${PROJECT_NAME}
)

# Install include directory (if you have one)
install(DIRECTORY include/
  DESTINATION include
)

ament_package()