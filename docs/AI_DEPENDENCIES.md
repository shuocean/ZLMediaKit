# ZLMediaKit AI Vision 模块依赖安装指南

本文档提供 AI Vision 模块所需依赖的详细安装说明。

## 系统要求

- **操作系统**: Linux (Ubuntu 20.04+, CentOS 7+), macOS 10.15+
- **编译器**: GCC 7+, Clang 10+, MSVC 2019+
- **CMake**: 3.16+
- **显卡**: NVIDIA GPU with Compute Capability 6.0+ (推荐 7.0+)

## 核心依赖

### 1. CUDA Toolkit 11.8+ (GPU推理必需)

**Ubuntu/Debian:**

```bash
# 添加NVIDIA官方源
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-keyring_1.0-1_all.deb
sudo dpkg -i cuda-keyring_1.0-1_all.deb
sudo apt-get update

# 安装CUDA 11.8
sudo apt-get install cuda-11-8

# 配置环境变量
echo 'export PATH=/usr/local/cuda-11.8/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda-11.8/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

# 验证安装
nvcc --version
nvidia-smi
```

**CentOS/RHEL:**

```bash
# 下载并安装CUDA
wget https://developer.download.nvidia.com/compute/cuda/11.8.0/local_installers/cuda-repo-rhel7-11-8-local-11.8.0_520.61.05-1.x86_64.rpm
sudo rpm -i cuda-repo-rhel7-11-8-local-11.8.0_520.61.05-1.x86_64.rpm
sudo yum clean all
sudo yum install cuda-11-8

# 配置环境变量（同上）
```

**macOS:** (不支持CUDA，可使用CPU模式或Metal)

### 2. cuDNN 8.x (CUDA深度学习加速库)

**下载地址**: <https://developer.nvidia.com/cudnn>

```bash
# 下载对应CUDA版本的cuDNN (需要NVIDIA开发者账号)
# 假设下载了 cudnn-linux-x86_64-8.9.0.131_cuda11-archive.tar.xz

tar -xvf cudnn-linux-x86_64-8.9.0.131_cuda11-archive.tar.xz
cd cudnn-linux-x86_64-8.9.0.131_cuda11-archive

# 复制文件到CUDA目录
sudo cp include/cudnn*.h /usr/local/cuda-11.8/include
sudo cp lib/libcudnn* /usr/local/cuda-11.8/lib64
sudo chmod a+r /usr/local/cuda-11.8/include/cudnn*.h /usr/local/cuda-11.8/lib64/libcudnn*
```

### 3. ONNX Runtime 1.16+ (GPU版本)

**方式1: 使用预编译包 (推荐)**

```bash
# 下载GPU版本
cd /opt
sudo wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-gpu-1.16.3.tgz
sudo tar -xzf onnxruntime-linux-x64-gpu-1.16.3.tgz
sudo mv onnxruntime-linux-x64-gpu-1.16.3 onnxruntime

# 设置环境变量
export ONNXRUNTIME_ROOT=/opt/onnxruntime
echo 'export ONNXRUNTIME_ROOT=/opt/onnxruntime' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=$ONNXRUNTIME_ROOT/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

**方式2: 使用包管理器**

```bash
# Ubuntu (需要PPA或手动安装)
# 目前官方仓库没有GPU版本，推荐使用方式1

# macOS
brew install onnxruntime
```

**验证安装:**

```bash
ls $ONNXRUNTIME_ROOT/include/onnxruntime_cxx_api.h
ls $ONNXRUNTIME_ROOT/lib/libonnxruntime.so
```

### 4. FFmpeg 4.4+ (CPU解码)

**Ubuntu/Debian:**

```bash
sudo apt-get install ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev
```

**CentOS/RHEL:**

```bash
# 启用EPEL和RPM Fusion
sudo yum install epel-release
sudo yum install --nogpgcheck https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
sudo yum install --nogpgcheck https://download1.rpmfusion.org/free/el/rpmfusion-free-release-7.noarch.rpm

sudo yum install ffmpeg ffmpeg-devel
```

**macOS:**

```bash
brew install ffmpeg
```

## 可选依赖

### 5. Python 3.8+ 和 pybind11 (Python插件功能)

**Ubuntu/Debian:**

```bash
sudo apt-get install python3 python3-dev python3-pip
pip3 install pybind11
```

**CentOS/RHEL:**

```bash
sudo yum install python3 python3-devel
pip3 install pybind11
```

**macOS:**

```bash
brew install python@3.11
pip3 install pybind11
```

### 6. 其他工具

```bash
# Git (版本管理)
sudo apt-get install git  # Ubuntu
sudo yum install git       # CentOS

# CMake 3.16+ (如果系统版本太旧)
wget https://github.com/Kitware/CMake/releases/download/v3.27.0/cmake-3.27.0-linux-x86_64.tar.gz
tar -xzf cmake-3.27.0-linux-x86_64.tar.gz
sudo mv cmake-3.27.0-linux-x86_64 /opt/cmake
echo 'export PATH=/opt/cmake/bin:$PATH' >> ~/.bashrc
```

## 快速安装脚本

**Ubuntu 20.04/22.04:**

```bash
#!/bin/bash
# 保存为 install_ai_deps.sh

set -e

echo "安装AI Vision模块依赖..."

# 更新包列表
sudo apt-get update

# 安装编译工具
sudo apt-get install -y build-essential cmake git

# 安装FFmpeg
sudo apt-get install -y ffmpeg libavcodec-dev libavformat-dev libavutil-dev libswscale-dev

# 安装Python和pybind11
sudo apt-get install -y python3 python3-dev python3-pip
pip3 install pybind11

# 提示安装CUDA和cuDNN
echo "请手动安装CUDA 11.8+和cuDNN 8.x (需要NVIDIA GPU)"
echo "参考: https://developer.nvidia.com/cuda-downloads"

# 下载ONNX Runtime
echo "正在下载ONNX Runtime..."
cd /tmp
wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-gpu-1.16.3.tgz
sudo tar -xzf onnxruntime-linux-x64-gpu-1.16.3.tgz -C /opt
sudo mv /opt/onnxruntime-linux-x64-gpu-1.16.3 /opt/onnxruntime

# 配置环境变量
echo 'export ONNXRUNTIME_ROOT=/opt/onnxruntime' | sudo tee -a /etc/profile.d/onnxruntime.sh
echo 'export LD_LIBRARY_PATH=$ONNXRUNTIME_ROOT/lib:$LD_LIBRARY_PATH' | sudo tee -a /etc/profile.d/onnxruntime.sh

echo "依赖安装完成！"
echo "请重新登录或执行: source /etc/profile.d/onnxruntime.sh"
```

## 验证安装

创建验证脚本 `check_deps.sh`:

```bash
#!/bin/bash

echo "检查AI Vision模块依赖..."

# 检查CUDA
if command -v nvcc &> /dev/null; then
    echo "✅ CUDA: $(nvcc --version | grep release | awk '{print $5}' | cut -c1-)"
else
    echo "❌ CUDA: 未安装"
fi

# 检查cuDNN
if [ -f "/usr/local/cuda/include/cudnn.h" ]; then
    echo "✅ cuDNN: 已安装"
else
    echo "❌ cuDNN: 未安装"
fi

# 检查ONNX Runtime
if [ -d "$ONNXRUNTIME_ROOT" ]; then
    echo "✅ ONNX Runtime: $ONNXRUNTIME_ROOT"
else
    echo "❌ ONNX Runtime: 未设置ONNXRUNTIME_ROOT"
fi

# 检查FFmpeg
if command -v ffmpeg &> /dev/null; then
    echo "✅ FFmpeg: $(ffmpeg -version | head -n1 | awk '{print $3}')"
else
    echo "❌ FFmpeg: 未安装"
fi

# 检查Python
if command -v python3 &> /dev/null; then
    echo "✅ Python: $(python3 --version | awk '{print $2}')"
else
    echo "❌ Python: 未安装"
fi

# 检查pybind11
if python3 -c "import pybind11" 2>/dev/null; then
    echo "✅ pybind11: 已安装"
else
    echo "❌ pybind11: 未安装"
fi

# 检查GPU
if command -v nvidia-smi &> /dev/null; then
    echo "✅ NVIDIA GPU:"
    nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv,noheader | head -n1
else
    echo "⚠️  nvidia-smi: 未找到 (CPU模式)"
fi

echo ""
echo "依赖检查完成！"
```

运行验证：

```bash
chmod +x check_deps.sh
./check_deps.sh
```

## 编译ZLMediaKit AI模块

```bash
cd ZLMediaKit
mkdir build && cd build

cmake .. \
  -DENABLE_AI_VISION=ON \
  -DENABLE_CUDA=ON \
  -DENABLE_PYTHON_PLUGIN=ON \
  -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)
```

## 常见问题

### Q1: CUDA找不到

```bash
# 确保CUDA路径正确
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH
```

### Q2: ONNX Runtime链接错误

```bash
# 确保LD_LIBRARY_PATH包含ONNX Runtime库
export LD_LIBRARY_PATH=$ONNXRUNTIME_ROOT/lib:$LD_LIBRARY_PATH
# 或者设置rpath
cmake .. -DCMAKE_INSTALL_RPATH=$ONNXRUNTIME_ROOT/lib
```

### Q3: 没有NVIDIA GPU，能用CPU模式吗？

可以！编译时关闭CUDA：

```bash
cmake .. -DENABLE_AI_VISION=ON -DENABLE_CUDA=OFF
```

### Q4: 显存不足怎么办？

参考 `UpgradePlan.md` 中的"GPU/CPU混合调度策略"，系统会自动降级到CPU推理。

## 下一步

- 阅读 [AI架构文档](AI_ARCHITECTURE.md)
- 查看 [集成指南](INTEGRATION_GUIDE.md)
- 参考 [升级计划](../UpgradePlan.md)

## 技术支持

- GitHub Issues: <https://github.com/ZLMediaKit/ZLMediaKit/issues>
- QQ群: 请参考README
