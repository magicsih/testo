cmake_minimum_required (VERSION 3.11)

set (OpenCV_DIR "D:/Development/opencv/opencv-4.1.1/build")
set (V8_INCLUDE_DIR "D:/Repositories/google/v8/include")
set (V8_LIBRARY_DIR "D:/Repositories/google/v8/out/x64.release")
set (V8_REQUIRED_COMPONENTS "v8;v8_libplatform;icui18n;icuuc;v8_libbase")
set (spdlog_DIR "C:/Workspace/VsCodeProjects/testo/spdlog/build")

# 이 프로젝트의 실행 파일에 소스를 추가합니다.
add_executable (testo 
					"testo.cpp" "testo.hpp" 
					"image_recognizer.cpp" "image_recognizer.hpp" 
					"adb_host_client.cpp" "adb_host_client.hpp" 
					"adb_util.cpp" "adb_util.hpp" 
					"adb_local_keep_alive_client.cpp" "adb_local_keep_alive_client.hpp"
					"adb_local_client.cpp" "adb_local_client.hpp"
					"image_loader.cpp" "image_loader.hpp"
					"testo_agent.cpp" "testo_agent.hpp"
				)

set_target_properties(testo PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

# BOOST ASIO
# https://cmake.org/cmake/help/latest/module/FindBoost.html
if(WIN32)
	add_definitions(-D_WIN32_WINNT=0x0501)
endif()

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)

find_package(Boost 1.71.0 COMPONENTS system regex date_time filesystem program_options REQUIRED)
target_link_libraries(testo ${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIRS})

# OPEN CV 
find_package( OpenCV COMPONENTS core imgcodecs imgproc REQUIRED)
target_link_libraries(testo ${OpenCV_LIBS})

if(WIN32)
	MACRO (copy_opencv_dll trgt libname)

		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			SET(OPENCV_DLL_NAME ${libname}d)
			SET(OPENCV_DLL_PATH "Debug")
		else()
			SET(OPENCV_DLL_NAME ${libname})
			Set(OPENCV_DLL_PATH "Release")
		endif()

		message (STATUS "COPY ${OpenCV_DIR}/bin/${OPENCV_DLL_PATH}/${OPENCV_DLL_NAME}.dll $<TARGET_FILE_DIR:testo>")		

		add_custom_command(TARGET ${trgt} POST_BUILD 
						COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"${OpenCV_DIR}/bin/${OPENCV_DLL_PATH}/${OPENCV_DLL_NAME}.dll"
						#"${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/${OPENCV_DLL_NAME}.dll")
						"$<TARGET_FILE_DIR:testo>")

	ENDMACRO()

	# 세미콜론으로 분리된 스트링, 세미콜론으로 스플릿해서 루프를 돈다.
	foreach(OPENCV_LIB ${OpenCV_LIBS})		
		copy_opencv_dll(${CMAKE_PROJECT_NAME} ${OPENCV_LIB}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH})
	endforeach()
endif()


#spdlog

include_directories("../spdlog/include")
#find_package(spdlog REQUIRED)
#target_link_libraries(testo spdlog::spdlog)

# v8
SET(V8_LIBS "")
foreach(V8_COMPONENT ${V8_REQUIRED_COMPONENTS})
	list(APPEND V8_LIBS "${V8_LIBRARY_DIR}/${V8_COMPONENT}.dll.lib")
endforeach()

message(STATUS "V8_LIBS:${V8_LIBS}")

include_directories("${V8_INCLUDE_DIR}")
target_link_libraries(testo "${V8_LIBS}")

add_custom_command(TARGET testo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${V8_LIBRARY_DIR}/natives_blob.bin" "$<TARGET_FILE_DIR:testo>")
add_custom_command(TARGET testo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${V8_LIBRARY_DIR}/snapshot_blob.bin" "$<TARGET_FILE_DIR:testo>")


if(WIN32)
	macro (copy_v8_dll trgt dll_name)
		
		message(STATUS "COPY ${V8_LIBRARY_DIR}/${dll_name}.dll ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/${dll_name}.dll")
		
		add_custom_command(TARGET ${trgt} POST_BUILD
						 COMMAND ${CMAKE_COMMAND} -E copy_if_different
						 "${V8_LIBRARY_DIR}/${dll_name}.dll"
						 #"${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/${dll_name}.dll"
						 "$<TARGET_FILE_DIR:testo>"
						 )
	endmacro()

	foreach(V8_DLL_NAME ${V8_REQUIRED_COMPONENTS})
		copy_v8_dll(${CMAKE_PROJECT_NAME} ${V8_DLL_NAME})
	endforeach()
	
endif()