# Final_AIServer

此项目为为实验室项目开发深度学习模型部署的demo

使用gRPC、LibTorch开发和C++11开发（LibTorch的编译需要C++11，但项目整体依据C++11的特性）

## LibTorch的编译

- 下载源码包（这里使用的LibTorch2.0 CPU版本）
- 解压到安装目录

## gRPC的编译

- 参见官方

## 项目编译

```shell
cd ${PROJECT_DIR} #进入项目根目录
mkdir build
pushd build
cmake -DCMAKE_PREFIX_PATH=${GPRC_INSTALL_DIR} ../
make -j 4
```

## 项目启动

```shell
./Final_AIServer ../model/model.pt
```

