http2.0

带了网络框架，异步同步，如果原生TCP，要自己实现reactor

# gRPC的下载和编译

## 方案一

https://pj-x86.github.io/2020/grpc%E6%BA%90%E7%A0%81%E4%B8%8B%E8%BD%BD%E5%8A%A0%E9%80%9F/

大体上根据这篇博客来的

- 可通过 gitee.com 先下载 grpc 主框架源码

  ```shell
  git clone git@gitee.com:mirrors/grpc.git
  ```

  ​

- 修改 grpc 依赖的第三方库下载地址。进入前面下载的 grpc 主框架源码目录，修改 .gitmodules ，只需修改其中的 url 参数，其他保持不变，注意并非每个第三方库都在 gitee.com 上有镜像仓库，没有的保持不变。参考如下：

  ```
  [submodule "third_party/zlib"]
          path = third_party/zlib
          #url = https://github.com/madler/zlib.git
          url = https://gitee.com/mirrors/zlib.git
          # When using CMake to build, the zlib submodule ends up with a
          # generated file that makes Git consider the submodule dirty. This
          # state can be ignored for day-to-day development on gRPC.
          ignore = dirty
  [submodule "third_party/protobuf"]
          path = third_party/protobuf
          url = https://github.com/google/protobuf.git
          #url = https://gitee.com/githubplus/protobuf.git
  [submodule "third_party/gflags"]
          path = third_party/gflags
          # url = https://github.com/gflags/gflags.git
          url = https://gitee.com/mirrors/gflags.git
  [submodule "third_party/googletest"]
          path = third_party/googletest
          # url = https://github.com/google/googletest.git
          url = https://gitee.com/mirrors/googletest.git
  [submodule "third_party/benchmark"]
          path = third_party/benchmark
          url = https://github.com/google/benchmark
  [submodule "third_party/boringssl-with-bazel"]
          path = third_party/boringssl-with-bazel
          # url = https://github.com/google/boringssl.git
          url = https://gitee.com/mirrors/boringssl.git
  [submodule "third_party/cares/cares"]
          path = third_party/cares/cares
          # url = https://github.com/c-ares/c-ares.git
          url = https://gitee.com/mirrors/c-ares.git
          branch = cares-1_12_0
  [submodule "third_party/bloaty"]
          path = third_party/bloaty
          # url = https://github.com/google/bloaty.git
          url = https://gitee.com/githubplus/bloaty.git
  [submodule "third_party/abseil-cpp"]
          path = third_party/abseil-cpp
          # url = https://github.com/abseil/abseil-cpp.git
          url = https://gitee.com/mirrors/abseil-cpp.git
          branch = lts_2020_02_25
  [submodule "third_party/envoy-api"]
          path = third_party/envoy-api
          url = https://github.com/envoyproxy/data-plane-api.git
          #url = https://gitee.com/githubplus/data-plane-api.git
  [submodule "third_party/googleapis"]
          path = third_party/googleapis
          # url = https://github.com/googleapis/googleapis.git
          url = https://gitee.com/mirrors/googleapis.git
  [submodule "third_party/protoc-gen-validate"]
          path = third_party/protoc-gen-validate
          # url = https://github.com/envoyproxy/protoc-gen-validate.git
          url = https://gitee.com/mirrors/protoc-gen-validate.git
  [submodule "third_party/udpa"]
          path = third_party/udpa
          url = https://github.com/cncf/udpa.git
  [submodule "third_party/libuv"]
          path = third_party/libuv
          # url = https://github.com/libuv/libuv.git
          url = https://gitee.com/mirrors/libuv.git
  ```

  这里的实例是根据这篇博客上来的，我自己使用的版本在源文件中。

- 更新 git submodule 配置。

  ```
  git submodule sync
  ```

- 更新第三方库源码

  ```
      git submodule update --init
  ```

- 这一步仍然会报很多错误，如果是下载的错误可以修改.gitmoduls的url信息，从码云上下载。另外，可以进入到某一个第三方库中，根据它的的依赖去下载。这时候要看一下，如果是已经下载过得第三方库可以直接复制过去。

## 方案二

科学上网

- 使用方案一仍然会遇到很多问题

```shell
fatal: unable to access 'https://github.com/xxx/autowrite.git/': 
OpenSSL SSL_read: Connection was reset, errno 10054
# 或者
fatal: unable to access 'https://github.com/xxx/autowrite.git/':
Failed to connect to github.com port 443: Timed out

```

- 究极方案，科学上网，使用全局代理模型
- 取消ssl验证：git config --global http.sslVerify false，这一步仿佛没有什么卵用
- git submodule update --init --recursive 递归的下载完所有第三方库，根据报错提示以及手动检查，把没有下载的再手动git clone一下