cmake_minimum_required(VERSION 3.26)

# --- 1. 配置依赖路径 ---
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(MHY_Scanner VERSION 1.1.16 LANGUAGES CXX)

# ✅ 禁用 vcpkg 自动复制 DLL（避免 post-build 调用 vcpkg.exe）
set(VCPKG_APPLOCAL_DEPS OFF CACHE BOOL "" FORCE)
set(VCPKG_MANIFEST_MODE OFF CACHE BOOL "" FORCE)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UNIT_TESTS "Unit Tests" OFF)
option(DEV "DEV" OFF)

add_definitions(-DMHY_Scanner_VERSION="${PROJECT_VERSION}")
# ✅ 宽字符/UNICODE 构建（TEXT() 宏展开为 L""，匹配代码中的 WCHAR 用法）
add_definitions(-DUNICODE -D_UNICODE)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    # ✅ Qt 6.11.1 编译必须选项
    add_compile_options(/Zi /sdl /arch:AVX2 /utf-8 /Zc:__cplusplus)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_link_options(/DEBUG /OPT:REF,ICF,LBR)
    endif()
endif()

include(FetchContent)
include(ExternalProject)

# 引入 FFmpeg 和 QR-Code 的配置脚本
include(cmake/install_ffmpeg.cmake)
include(cmake/build_QR-Code-generator.cmake)

# --- 2. 本地化 nlohmann_json ---
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${CMAKE_SOURCE_DIR}/3rdparty/nlohmann/single_include)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)

# --- 3. 查找非 Qt 依赖 ---
find_package(unofficial-webview2 CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

# OpenCV 手动配置
set(OPENCV_ROOT "D:/opencv_build/install")
include_directories("${OPENCV_ROOT}/include")
set(MANUAL_OPENCV_LIBS
    "${OPENCV_ROOT}/x64/vc17/lib/opencv_core4120.lib"
    "${OPENCV_ROOT}/x64/vc17/lib/opencv_imgproc4120.lib"
    "${OPENCV_ROOT}/x64/vc17/lib/opencv_objdetect4120.lib"
    "${OPENCV_ROOT}/x64/vc17/lib/opencv_wechat_qrcode4120.lib"
    "${OPENCV_ROOT}/x64/vc17/lib/opencv_imgcodecs4120.lib"
    "${OPENCV_ROOT}/x64/vc17/lib/opencv_videoio4120.lib"
    "${OPENCV_ROOT}/x64/vc17/lib/opencv_highgui4120.lib"
    "${OPENCV_ROOT}/x64/vc17/lib/opencv_dnn4120.lib"
)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

# --- 4. 硬编码本地 Qt + 手动启用 MOC/UIC/RCC ---
set(QT_LOCAL_ROOT "D:/Qt/6.11.1/msvc2022_64")

# ✅ 手动指定 Qt 工具链路径
set(CMAKE_AUTOMOC_EXECUTABLE "${QT_LOCAL_ROOT}/bin/moc.exe")
set(CMAKE_AUTOUIC_EXECUTABLE "${QT_LOCAL_ROOT}/bin/uic.exe")
set(CMAKE_AUTORCC_EXECUTABLE "${QT_LOCAL_ROOT}/bin/rcc.exe")

# ✅ 强制重新启用 Qt 自动生成器
set(CMAKE_AUTOUIC ON CACHE BOOL "" FORCE)
set(CMAKE_AUTOMOC ON CACHE BOOL "" FORCE)
set(CMAKE_AUTORCC ON CACHE BOOL "" FORCE)

# ✅ 显式指定 UI 文件搜索路径
set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_SOURCE_DIR}/src/UI" CACHE PATH "" FORCE)

# 添加头文件搜索路径
include_directories(
    "${QT_LOCAL_ROOT}/include"
    "${QT_LOCAL_ROOT}/include/QtCore"
    "${QT_LOCAL_ROOT}/include/QtGui"
    "${QT_LOCAL_ROOT}/include/QtWidgets"
)

# 创建导入库目标
add_library(Qt6::Core SHARED IMPORTED GLOBAL)
set_target_properties(Qt6::Core PROPERTIES
    IMPORTED_LOCATION "${QT_LOCAL_ROOT}/bin/Qt6Core.dll"
    IMPORTED_IMPLIB "${QT_LOCAL_ROOT}/lib/Qt6Core.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${QT_LOCAL_ROOT}/include;${QT_LOCAL_ROOT}/include/QtCore"
)

add_library(Qt6::Gui SHARED IMPORTED GLOBAL)
set_target_properties(Qt6::Gui PROPERTIES
    IMPORTED_LOCATION "${QT_LOCAL_ROOT}/bin/Qt6Gui.dll"
    IMPORTED_IMPLIB "${QT_LOCAL_ROOT}/lib/Qt6Gui.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${QT_LOCAL_ROOT}/include;${QT_LOCAL_ROOT}/include/QtGui"
    INTERFACE_LINK_LIBRARIES "Qt6::Core"
)

add_library(Qt6::Widgets SHARED IMPORTED GLOBAL)
set_target_properties(Qt6::Widgets PROPERTIES
    IMPORTED_LOCATION "${QT_LOCAL_ROOT}/bin/Qt6Widgets.dll"
    IMPORTED_IMPLIB "${QT_LOCAL_ROOT}/lib/Qt6Widgets.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${QT_LOCAL_ROOT}/include;${QT_LOCAL_ROOT}/include/QtWidgets"
    INTERFACE_LINK_LIBRARIES "Qt6::Core;Qt6::Gui"
)

# Boost (本地路径)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/boost)

if(NOT DEV)
    add_definitions(-DTESTSPEED)
    add_definitions(-DSHOW)
endif()

include_directories(src/Core)

add_executable(${PROJECT_NAME}
    "${CMAKE_SOURCE_DIR}/src/Resources/MHY_Scanner.rc"
    "${CMAKE_SOURCE_DIR}/src/Resources/MHY_Scanner.qrc" 
    "${CMAKE_SOURCE_DIR}/src/UI/WindowMain.ui"
    "${CMAKE_SOURCE_DIR}/src/UI/Main.cpp" 
    "${CMAKE_SOURCE_DIR}/src/UI/QRCodeForStream.cpp"
    "${CMAKE_SOURCE_DIR}/src/UI/QRCodeForStream.h"
    "${CMAKE_SOURCE_DIR}/src/UI/QRCodeForScreen.cpp" 
    "${CMAKE_SOURCE_DIR}/src/UI/QRCodeForScreen.h" 
    "${CMAKE_SOURCE_DIR}/src/UI/WindowAbout.cpp" 
    "${CMAKE_SOURCE_DIR}/src/UI/WindowAbout.h"
    "${CMAKE_SOURCE_DIR}/src/UI/WindowMain.cpp" 
    "${CMAKE_SOURCE_DIR}/src/UI/WindowMain.h" 
    "${CMAKE_SOURCE_DIR}/src/UI/WindowLogin.h"
    "${CMAKE_SOURCE_DIR}/src/UI/WindowLogin.cpp" 
    "${CMAKE_SOURCE_DIR}/src/UI/WindowGeeTest.h" 
    "${CMAKE_SOURCE_DIR}/src/UI/WindowGeeTest.cpp"
    "${CMAKE_SOURCE_DIR}/src/Core/CryptoKit.cpp"
    "${CMAKE_SOURCE_DIR}/src/Core/CryptoKit.h"
    "${CMAKE_SOURCE_DIR}/src/Core/LiveStreamLink.cpp"
    "${CMAKE_SOURCE_DIR}/src/Core/LiveStreamLink.h"
    "${CMAKE_SOURCE_DIR}/src/Core/QRScanner.cpp"
    "${CMAKE_SOURCE_DIR}/src/Core/QRScanner.h"
    "${CMAKE_SOURCE_DIR}/src/Core/ScreenScan.cpp"
    "${CMAKE_SOURCE_DIR}/src/Core/ScreenScan.h"
    "${CMAKE_SOURCE_DIR}/src/Core/ConfigDate.cpp"
    "${CMAKE_SOURCE_DIR}/src/Core/ConfigDate.h"
    "${CMAKE_SOURCE_DIR}/src/Core/BSGameSDK.hpp"
    "${CMAKE_SOURCE_DIR}/src/Core/CookieParser.hpp"
    "${CMAKE_SOURCE_DIR}/src/Core/CreateUUID.hpp"
    "${CMAKE_SOURCE_DIR}/src/Core/ScannerBase.hpp"
    "${CMAKE_SOURCE_DIR}/src/Core/compile_string.hpp"
    "${CMAKE_SOURCE_DIR}/src/Core/ApiDefs.hpp"
)

if(NOT DEV)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# ✅ 强制使用控制台子系统
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")

set(PROJECT_LIBRARIES
    Ws2_32 wldap32 Normaliz Dbghelp dxgi
    Qt6::Core Qt6::Gui Qt6::Widgets
    cpr::cpr
    QR_Code_generator
    unofficial::webview2::webview2
    nlohmann_json::nlohmann_json
    ${MANUAL_OPENCV_LIBS}
    ${FFMPEG_LIBS}
    OpenSSL::Crypto
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIBRARIES})

# --- 5. 禁用 vcpkg 自动复制 DLL + 手动复制所有依赖 ---
set(VCPKG_APPLOCAL_DEPS OFF CACHE BOOL "" FORCE)

# 收集 OpenCV / FFmpeg DLL
file(GLOB OPENCV_DLLS "${OPENCV_ROOT}/x64/vc17/bin/*.dll")
file(GLOB FFMPEG_DLL_FILES
    "${FFMPEG_BIN_DIR}/avcodec-*.dll"
    "${FFMPEG_BIN_DIR}/avformat-*.dll"
    "${FFMPEG_BIN_DIR}/avutil-*.dll"
    "${FFMPEG_BIN_DIR}/swscale-*.dll"
    "${FFMPEG_BIN_DIR}/swresample-*.dll"
)

# 本地 Qt 核心 DLL
set(LOCAL_QT_CORE_DLLS
    "${QT_LOCAL_ROOT}/bin/Qt6Core.dll"
    "${QT_LOCAL_ROOT}/bin/Qt6Gui.dll"
    "${QT_LOCAL_ROOT}/bin/Qt6Widgets.dll"
    "${QT_LOCAL_ROOT}/bin/Qt6Network.dll"
    "${QT_LOCAL_ROOT}/bin/Qt6Svg.dll"
    "${QT_LOCAL_ROOT}/bin/D3Dcompiler_47.dll"
)

# vcpkg 第三方依赖 DLL
set(VCPKG_THIRD_PARTY_DLLS
    "D:/vcpkg/installed/x64-windows/bin/cpr.dll"
    "D:/vcpkg/installed/x64-windows/bin/libcurl.dll"
    "D:/vcpkg/installed/x64-windows/bin/libcrypto-3-x64.dll"
    "D:/vcpkg/installed/x64-windows/bin/libssl-3-x64.dll"
    "D:/vcpkg/installed/x64-windows/bin/icuin78.dll"
    "D:/vcpkg/installed/x64-windows/bin/icuuc78.dll"
    "D:/vcpkg/installed/x64-windows/bin/icudt78.dll"
    "D:/vcpkg/installed/x64-windows/bin/z.dll"
    "D:/vcpkg/installed/x64-windows/bin/pcre2-16.dll"
    "D:/vcpkg/installed/x64-windows/bin/harfbuzz.dll"
    "D:/vcpkg/installed/x64-windows/bin/freetype.dll"
    "D:/vcpkg/installed/x64-windows/bin/libpng16.dll"
    "D:/vcpkg/installed/x64-windows/bin/md4c.dll"
    "D:/vcpkg/installed/x64-windows/bin/double-conversion.dll"
    "D:/vcpkg/installed/x64-windows/bin/bz2.dll"
    "D:/vcpkg/installed/x64-windows/bin/brotlidec.dll"
    "D:/vcpkg/installed/x64-windows/bin/brotlicommon.dll"
    "D:/vcpkg/installed/x64-windows/bin/zstd.dll"
)

find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_LOCAL_ROOT}/bin" REQUIRED)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    # 【步骤1】清理旧的 Qt 文件和插件
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_FILE_DIR:${PROJECT_NAME}>/styles"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Qt6Core.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Qt6Gui.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Qt6Widgets.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Qt6Network.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Qt6Svg.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/D3Dcompiler_47.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icuin78.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icuuc78.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/icudt78.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/z.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/pcre2-16.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/harfbuzz.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/freetype.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libpng16.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/md4c.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/double-conversion.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/bz2.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/brotlidec.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/brotlicommon.dll"
    COMMAND ${CMAKE_COMMAND} -E rm -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/zstd.dll"
    
    # 【步骤2】复制 OpenCV
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OPENCV_DLLS} "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    
    # 【步骤3】复制 FFmpeg
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FFMPEG_DLL_FILES} "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    
    # 【步骤4】复制本地 Qt 核心 DLL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LOCAL_QT_CORE_DLLS} "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    
    # 【步骤5】复制 vcpkg 第三方依赖 DLL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${VCPKG_THIRD_PARTY_DLLS} "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    
    # 【步骤6】用 windeployqt 复制平台插件和样式
    COMMAND ${WINDEPLOYQT_EXECUTABLE} 
        "$<TARGET_FILE:${PROJECT_NAME}>" 
        --no-translations 
        --no-opengl-sw
        --no-compiler-runtime
        --verbose 2
        
    COMMENT "Deploying local Qt core(6) + vcpkg third-party deps + OpenCV + FFmpeg..."
)

# Unit tests
if(UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
    target_link_libraries(test_core PRIVATE ${PROJECT_LIBRARIES})
    target_link_libraries(test_gui PRIVATE ${PROJECT_LIBRARIES})
    target_link_libraries(test_video PRIVATE ${PROJECT_LIBRARIES})
    target_link_libraries(test_video_hw PRIVATE ${PROJECT_LIBRARIES})
    target_link_libraries(test_streamlink PRIVATE ${PROJECT_LIBRARIES})
endif()

# Install rules
set(Install_Directory "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}_${PROJECT_VERSION}")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${Install_Directory})
install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${Install_Directory})

install(FILES 
    "${QT_LOCAL_ROOT}/bin/Qt6Core.dll"
    "${QT_LOCAL_ROOT}/bin/Qt6Widgets.dll"
    "${QT_LOCAL_ROOT}/bin/Qt6Gui.dll"
    ${FFMPEG_DLL_FILES}
    DESTINATION ${Install_Directory}
)

install(FILES
    "${QT_LOCAL_ROOT}/plugins/platforms/qdirect2d.dll"
    "${QT_LOCAL_ROOT}/plugins/platforms/qminimal.dll"
    "${QT_LOCAL_ROOT}/plugins/platforms/qoffscreen.dll"
    "${QT_LOCAL_ROOT}/plugins/platforms/qwindows.dll"
    DESTINATION ${Install_Directory}/plugins/platforms
)

install(FILES "${QT_LOCAL_ROOT}/plugins/styles/qmodernwindowsstyle.dll" DESTINATION ${Install_Directory}/plugins/styles)
install(DIRECTORY ScanModel DESTINATION ${Install_Directory})
