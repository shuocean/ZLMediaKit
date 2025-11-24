# ZLMediaKit AI Vision Module
# 注意：使用条件编译，最小化影响现有构建系统

cmake_minimum_required(VERSION 3.16)

# ================== 编译选项 ==================
# 这些选项在根CMakeLists.txt中定义，这里使用
message(STATUS "AI Vision Module Configuration:")
message(STATUS "  ENABLE_AI_VISION: ${ENABLE_AI_VISION}")
message(STATUS "  ENABLE_CUDA: ${ENABLE_CUDA}")
message(STATUS "  ENABLE_PYTHON_PLUGIN: ${ENABLE_PYTHON_PLUGIN}")

# ================== 查找依赖 ==================

# 1. CUDA（GPU推理必需）
if(ENABLE_CUDA)
    find_package(CUDAToolkit REQUIRED)
    if(CUDAToolkit_FOUND)
        message(STATUS "  CUDA Version: ${CUDAToolkit_VERSION}")
        message(STATUS "  CUDA Include: ${CUDAToolkit_INCLUDE_DIRS}")
    else()
        message(FATAL_ERROR "CUDA not found but ENABLE_CUDA=ON")
    endif()
endif()

# 2. ONNX Runtime（AI推理核心）
# 注意：这里使用自定义的FindOnnxRuntime.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
find_package(OnnxRuntime)
if(OnnxRuntime_FOUND)
    message(STATUS "  ONNX Runtime: ${OnnxRuntime_INCLUDE_DIRS}")
    message(STATUS "  ONNX Runtime Libs: ${OnnxRuntime_LIBRARIES}")
else()
    # 如果找不到，尝试从环境变量查找
    if(DEFINED ENV{ONNXRUNTIME_ROOT})
        set(OnnxRuntime_INCLUDE_DIRS $ENV{ONNXRUNTIME_ROOT}/include)
        set(OnnxRuntime_LIBRARIES $ENV{ONNXRUNTIME_ROOT}/lib/libonnxruntime.so)
        message(STATUS "  ONNX Runtime from ENV: ${OnnxRuntime_INCLUDE_DIRS}")
    else()
        message(WARNING "ONNX Runtime not found, please set ONNXRUNTIME_ROOT env variable")
    endif()
endif()

# 3. Python + pybind11（插件功能，可选）
if(ENABLE_PYTHON_PLUGIN)
    find_package(Python3 COMPONENTS Interpreter Development)
    if(Python3_FOUND)
        message(STATUS "  Python3: ${Python3_VERSION}")
    endif()
    
    find_package(pybind11 CONFIG)
    if(pybind11_FOUND)
        message(STATUS "  pybind11: ${pybind11_VERSION}")
    else()
        message(WARNING "pybind11 not found, Python plugin disabled")
        set(ENABLE_PYTHON_PLUGIN OFF)
    endif()
endif()

# ================== 源文件收集 ==================

# 核心源文件（始终编译）
set(AI_VISION_SOURCES
    src/core/InferenceEngine.cpp
    src/core/ModelRegistry.cpp
    src/core/DetectionResult.cpp
    src/core/FrameConverter.cpp
    src/core/GpuUploader.cpp
    src/core/AIPipeline.cpp
    src/core/YoloDetector.cpp
    src/core/JsonHelper.cpp
    src/task/AITaskManager.cpp
    src/task/TaskScheduler.cpp
    src/alert/AlertEngine.cpp
    src/alert/AlertRule.cpp
    src/alert/WebHookNotifier.cpp
    src/api/AIApiController.cpp
)

# Python插件源文件（条件编译）
if(ENABLE_PYTHON_PLUGIN)
    list(APPEND AI_VISION_SOURCES
        src/plugin/PythonBridge.cpp
        src/plugin/PythonPluginManager.cpp
    )
endif()

# ================== 创建库目标 ==================

add_library(ai-vision STATIC ${AI_VISION_SOURCES})

# 创建别名供其他模块使用
add_library(ZLMediaKit::ai-vision ALIAS ai-vision)

# ================== 头文件路径 ==================

target_include_directories(ai-vision
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/zlmediakit/ai-vision>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 添加ONNX Runtime头文件
if(OnnxRuntime_INCLUDE_DIRS)
    target_include_directories(ai-vision PRIVATE ${OnnxRuntime_INCLUDE_DIRS})
endif()

# 添加CUDA头文件
if(ENABLE_CUDA AND CUDAToolkit_FOUND)
    target_include_directories(ai-vision PRIVATE ${CUDAToolkit_INCLUDE_DIRS})
endif()

# ================== 链接库 ==================

target_link_libraries(ai-vision
    PUBLIC
        ZLMediaKit::MediaKit
        ZLMediaKit::ToolKit
    PRIVATE
        ${OnnxRuntime_LIBRARIES}
)

# 链接CUDA库
if(ENABLE_CUDA AND CUDAToolkit_FOUND)
    target_link_libraries(ai-vision PRIVATE CUDA::cudart)
endif()

# 链接Python库
if(ENABLE_PYTHON_PLUGIN AND pybind11_FOUND)
    target_link_libraries(ai-vision PRIVATE pybind11::embed)
endif()

# ================== 编译选项 ==================

# C++17标准（与主项目保持一致）
target_compile_features(ai-vision PUBLIC cxx_std_17)

# 编译定义
target_compile_definitions(ai-vision
    PUBLIC
        ENABLE_AI_VISION  # 公开定义，供其他模块使用
    PRIVATE
        $<$<BOOL:${ENABLE_CUDA}>:ENABLE_CUDA>
        $<$<BOOL:${ENABLE_PYTHON_PLUGIN}>:ENABLE_PYTHON_PLUGIN>
)

# 编译选项（优化性能）
if(CMAKE_BUILD_TYPE MATCHES "Release")
    target_compile_options(ai-vision PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-O3>
        $<$<CXX_COMPILER_ID:GNU,Clang>:-march=native>  # CPU优化
        $<$<CXX_COMPILER_ID:MSVC>:/O2>
    )
endif()

# ================== 安装配置 ==================

# 安装头文件
install(DIRECTORY include/
    DESTINATION include/zlmediakit/ai-vision
    FILES_MATCHING PATTERN "*.h"
)

# 安装库文件
install(TARGETS ai-vision
    EXPORT ZLMediaKitTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# 安装Python SDK（可选）
if(ENABLE_PYTHON_PLUGIN)
    install(DIRECTORY python/
        DESTINATION share/zlmediakit/ai-vision/python
        FILES_MATCHING PATTERN "*.py"
    )
endif()

# ================== 模块信息输出 ==================

message(STATUS "AI Vision Module Build Configuration Summary:")
message(STATUS "  Library: ai-vision (STATIC)")
message(STATUS "  Sources: ${AI_VISION_SOURCES}")
message(STATUS "  CUDA Support: ${ENABLE_CUDA}")
message(STATUS "  Python Plugin: ${ENABLE_PYTHON_PLUGIN}")
message(STATUS "-----------------------------------------------")
