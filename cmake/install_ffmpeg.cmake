# --- 本地化 FFmpeg 配置 (不再自动下载) ---

# 自动指向项目下的 3rdparty 文件夹
set(FFMPEG_DIR "${CMAKE_SOURCE_DIR}/3rdparty/ffmpeg-master-latest-win64-gpl-shared")

# 检查文件夹是否存在
if(NOT EXISTS ${FFMPEG_DIR})
    message(FATAL_ERROR "找不到本地的 FFmpeg 文件夹！请确认是否已解压到: ${FFMPEG_DIR}")
endif()

set(FFMPEG_LIBS
	avcodec
	avdevice
	avfilter
	avformat
	avutil
	swscale
	swresample
)

set(FFMPEG_BIN_DIR ${FFMPEG_DIR}/bin)
set(FFMPEG_LIB_DIR ${FFMPEG_DIR}/lib)
set(FFMPEG_INCLUDE_DIR ${FFMPEG_DIR}/include)

link_directories(${FFMPEG_LIB_DIR})
include_directories(${FFMPEG_INCLUDE_DIR})