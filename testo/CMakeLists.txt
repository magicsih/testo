# CMakeList.txt: testo에 대한 CMake 프로젝트, 여기에 소스를 포함하고
# 프로젝트 특정 논리를 정의합니다.
#
cmake_minimum_required (VERSION 3.11)

get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

if(WIN32)
	#BOOST
	SET(BOOST_ROOT "C:/Development/boost/boost_1_69_0")
	SET(BOOST_LIBRARYDIR, "${BOOST_ROOT}/stage/lib")
	add_definitions(-D_WIN32_WINNT=0x0501)

	#OPENCV
	set(OpenCV_DIR "C:/Development/OpenCV/opencv401_build")

	#V8
	SET(V8_INCLUDE_DIR "D:/Repositories/google/v8/include")
	SET(V8_LIBRARY_DIR "D:/Repositories/google/v8/out.gn/ia32.debug")
	SET(V8_REQUIRED_COMPONENTS "v8;v8_libplatform;icui18n;icuuc;v8_libbase")	
else()
	#TODO Linux Boost ROOT
	SET(BOOST_ROOT "")
	SET(BOOST_LIBRARYDIR, "")

	#OPENCV
	set(OpenCV_DIR "/usr/lib/opencv")
endif()

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
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREAD ON)

find_package(Boost 1.69.0 COMPONENTS system regex date_time filesystem program_options REQUIRED)

link_directories(${BOOST_LIBRARYDIR})
include_directories(${BOOST_ROOT} )

message(STATUS "Boost LINKING LIBRARIES\n${Boost_LIBRARIES}")
target_link_libraries(testo ${Boost_LIBRARIES})


# OPEN CV 
find_package( OpenCV COMPONENTS core imgcodecs imgproc REQUIRED)

message(STATUS "OpenCV INCLUDE_DIRS\n${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV LINKING LIBRARIES\n${OpenCV_LIBS}")

target_link_libraries(testo ${OpenCV_LIBS})

message(STATUS "OpenCV INSTALL PATH:${OpenCV_INSTALL_PATH}")
message(STATUS "CMAKE_BINARY_DIR:${CMAKE_BINARY_DIR}")

if(WIN32)
	#WIN32인 경우 OPENCV DLL 라이브러리들 MACRO 써서 카피

	MACRO (copy_opencv_dll trgt libname)

		if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			SET(OPENCV_DLL_NAME ${libname}d)
			SET(OPENCV_DLL_PATH "Debug")
		else()
			SET(OPENCV_DLL_NAME ${libname})
			Set(OPENCV_DLL_PATH "Release")
		endif()

		message (STATUS "COPY ${OpenCV_DIR}/bin/${OPENCV_DLL_PATH}/${OPENCV_DLL_NAME}.dll ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/${OPENCV_DLL_NAME}.dll")		

		add_custom_command(TARGET ${trgt} POST_BUILD 
						COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"${OpenCV_DIR}/bin/${OPENCV_DLL_PATH}/${OPENCV_DLL_NAME}.dll"
						"${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/${OPENCV_DLL_NAME}.dll")

	ENDMACRO()

	# 세미콜론으로 분리된 스트링, 세미콜론으로 스플릿해서 루프를 돈다.
	foreach(OPENCV_LIB ${OpenCV_LIBS})		
		copy_opencv_dll(${CMAKE_PROJECT_NAME} ${OPENCV_LIB}${OpenCV_VERSION_MAJOR}${OpenCV_VERSION_MINOR}${OpenCV_VERSION_PATCH})
	endforeach()
endif()


#spdlog
include_directories("../spdlog/include")

# v8
SET(V8_LIBS "")
foreach(V8_COMPONENT ${V8_REQUIRED_COMPONENTS})
	list(APPEND V8_LIBS "${V8_LIBRARY_DIR}/${V8_COMPONENT}.dll.lib")
endforeach()

message(STATUS "V8_LIBS:${V8_LIBS}")

include_directories("${V8_INCLUDE_DIR}")
target_link_libraries(testo "${V8_LIBS}")

add_custom_command(TARGET testo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${V8_LIBRARY_DIR}/natives_blob.bin" "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/.")
add_custom_command(TARGET testo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${V8_LIBRARY_DIR}/snapshot_blob.bin" "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/.")


if(WIN32)
	macro (copy_v8_dll trgt dll_name)
		
		message(STATUS "COPY ${V8_LIBRARY_DIR}/${dll_name}.dll ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/${dll_name}.dll")
		
		add_custom_command(TARGET ${trgt} POST_BUILD
						 COMMAND ${CMAKE_COMMAND} -E copy_if_different
						 "${V8_LIBRARY_DIR}/${dll_name}.dll"
						 "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}/${CMAKE_BUILD_TYPE}/${dll_name}.dll"
						 )
	endmacro()

	foreach(V8_DLL_NAME ${V8_REQUIRED_COMPONENTS})
		copy_v8_dll(${CMAKE_PROJECT_NAME} ${V8_DLL_NAME})
	endforeach()
	
endif()