# FindOnnxRuntime.cmake
# 查找ONNX Runtime库
# Find ONNX Runtime library
#
# 使用方法 / Usage:
#   find_package(OnnxRuntime)
#
# 定义变量 / Defines:
#   OnnxRuntime_FOUND - 是否找到ONNX Runtime
#   OnnxRuntime_INCLUDE_DIRS - 头文件目录
#   OnnxRuntime_LIBRARIES - 库文件
#   OnnxRuntime_VERSION - 版本号
#
# 环境变量 / Environment Variables:
#   ONNXRUNTIME_ROOT - ONNX Runtime安装根目录
#

# 查找头文件
# Find include directory
find_path(OnnxRuntime_INCLUDE_DIR
    NAMES onnxruntime_cxx_api.h onnxruntime_c_api.h
    PATHS
        ${ONNXRUNTIME_ROOT}
        $ENV{ONNXRUNTIME_ROOT}
        /usr/local
        /usr
        /opt/onnxruntime
    PATH_SUFFIXES
        include
        include/onnxruntime
        onnxruntime/include
)

# 查找库文件
# Find library
find_library(OnnxRuntime_LIBRARY
    NAMES onnxruntime
    PATHS
        ${ONNXRUNTIME_ROOT}
        $ENV{ONNXRUNTIME_ROOT}
        /usr/local
        /usr
        /opt/onnxruntime
    PATH_SUFFIXES
        lib
        lib64
        lib/x86_64-linux-gnu
        onnxruntime/lib
)

# 尝试获取版本号
# Try to get version
if(OnnxRuntime_INCLUDE_DIR)
    file(STRINGS "${OnnxRuntime_INCLUDE_DIR}/onnxruntime_c_api.h" 
         _onnxruntime_version_line
         REGEX "^#define ORT_API_VERSION")
    if(_onnxruntime_version_line)
        string(REGEX REPLACE "^#define ORT_API_VERSION ([0-9]+).*" "\\1" 
               OnnxRuntime_VERSION "${_onnxruntime_version_line}")
    endif()
endif()

# 处理标准参数
# Handle standard arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OnnxRuntime
    REQUIRED_VARS
        OnnxRuntime_LIBRARY
        OnnxRuntime_INCLUDE_DIR
    VERSION_VAR
        OnnxRuntime_VERSION
)

# 设置输出变量
# Set output variables
if(OnnxRuntime_FOUND)
    set(OnnxRuntime_INCLUDE_DIRS ${OnnxRuntime_INCLUDE_DIR})
    set(OnnxRuntime_LIBRARIES ${OnnxRuntime_LIBRARY})
    
    message(STATUS "Found ONNX Runtime:")
    message(STATUS "  Include: ${OnnxRuntime_INCLUDE_DIRS}")
    message(STATUS "  Library: ${OnnxRuntime_LIBRARIES}")
    if(OnnxRuntime_VERSION)
        message(STATUS "  Version: ${OnnxRuntime_VERSION}")
    endif()
    
    # 创建导入目标（可选）
    # Create imported target (optional)
    if(NOT TARGET OnnxRuntime::OnnxRuntime)
        add_library(OnnxRuntime::OnnxRuntime UNKNOWN IMPORTED)
        set_target_properties(OnnxRuntime::OnnxRuntime PROPERTIES
            IMPORTED_LOCATION "${OnnxRuntime_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OnnxRuntime_INCLUDE_DIR}"
        )
    endif()
endif()

# 标记为高级变量（在cmake-gui中隐藏）
# Mark as advanced (hide in cmake-gui)
mark_as_advanced(
    OnnxRuntime_INCLUDE_DIR
    OnnxRuntime_LIBRARY
)
