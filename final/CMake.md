cmake入门使用

https://zhuanlan.zhihu.com/p/534439206

https://zhuanlan.zhihu.com/p/500002865

# Step 1：构建最小的项目

```cmake
cmake_minimum_required(VERSION 3.15)

# set the project name
project(Tutorial)

# add the executable
add_executable(Tutorial tutorial.cpp)
```

`cmake_minimum_required`  指定使用 CMake 的最低版本号，`project` 指定项目名称，`add_executable` 用来生成可执行文件，需要指定生成可执行文件的名称和相关源文件。

```c++
// tutorial.cpp

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " number" << std::endl;
        return 1;
    }

    // convert input to double
    const double inputValue = atof(argv[1]);

    // calculate square root
    const double outputValue = sqrt(inputValue);
    std::cout << "The square root of " << inputValue
              << " is " << outputValue
              << std::endl;
    return 0;
}
```

## 构建、编译和运行

先从命令行进入到 step1 目录，并创建一个构建目录 build，接下来，进入 build 目录并运行 CMake 来配置项目，并生成构建系统：

```shell
mkdir build
cd build
cmake -G"MinGW Makefiles" ..
```

构建系统是需要指定 CMakeLists.txt 所在路径，此时在 build 目录下，所以用 `..` 表示 CMakeLists.txt 在上一级目录。

Windows 下，CMake 默认使用微软的 MSVC 作为编译器，我想使用 MinGW 编译器，可以通过 `-G` 参数来进行指定，只有第一次构建项目时需要指定。

此时在 build 目录下会生成 Makefile 文件，然后调用编译器来实际编译和链接项目：

```shell
cmake --build .
```

`--build` 指定编译生成的文件存放目录，其中就包括可执行文件，`.` 表示存放到当前目录，

## 外部构建与内部构建

这里创建了一个 build 目录存放编译产物，可以避免编译产物与代码文件混在一起，这种叫做外部构建。

还有一种内部构建，即直接在项目根目录下进行构建系统与编译，这时构建和编译命令就更改为：

```shell
cmake -G"MinGW Makefiles" .
cmake --build .
```

内部构建会使得项目文件很混乱，一般直接用外部构建即可。

# step 2：优化CMakelists.txt

## set与PROJECT_NAME

这是之前见过的 CMakeLists.txt 文件：

```cmake
cmake_minimum_required(VERSION 3.15)

# set the project name
project(Tutorial)

# add the executable
add_executable(Tutorial tutorial.cpp)
```

指定了项目名后，后面可能会有多个地方用到这个项目名，如果更改了这个名字，就要改多个地方，比较麻烦，那么可以使用 `PROJECT_NAME` 来表示项目名。

```cmake
add_executable(${PROJECT_NAME} tutorial.cpp)
```

生成可执行文件需要指定相关的源文件，如果有多个，那么就用空格隔开，比如：

```cmake
add_executable(${PROJECT_NAME} a.cpp b.cpp c.cpp)
```

我们也可以用一个变量来表示这多个源文件：

```cmake
set(SRC_LIST a.cpp b.cpp c.cpp)
add_executable(${PROJECT_NAME} ${SRC_LIST})
```

`set` 命令指定 SRC_LIST 变量来表示多个源文件，用 `${var_name}` 获取变量的值。

于是原来的 CMakeLists.txt 文件就可以变成如下所示：

```cmake
cmake_minimum_required(VERSION 3.15)

# set the project name
project(Tutorial)

SET(SRC_LIST tutorial.cpp)

# add the executable
add_executable(${PROJECT_NAME} ${SRC_LIST})
```

这样看起来就很简洁。

## 添加版本号和头文件

我们可以在 CMakeLists.txt 为可执行文件和项目提供一个版本号。首先，修改 CMakeLists.txt 文件，使用 project 命令设置项目名称和版本号。

```cmake
cmake_minimum_required(VERSION 3.15)

# set the project name and version
project(Tutorial VERSION 1.0.2)

configure_file(TutorialConfig.h.in TutorialConfig.h)
```

然后，配置头文件将版本号传递给源代码：

```cmake
configure_file(TutorialConfig.h.in TutorialConfig.h)
```

由于 TutorialConfig.h 文件这里被设置为自动写入 build 目录，因此需要将该目录添加到搜索头文件的路径列表中，也可以修改为写到其它目录。

将以下行添加到 CMakeLists.txt 文件的末尾：

```cmake
target_include_directories(${PROJECT_NAME} PUBLIC
                           ${PROJECT_BINARY_DIR}
                           )
```

`PROJECT_BINARY_DIR` 表示当前工程的二进制路径，即编译产物会存放到该路径，此时`PROJECT_BINARY_DIR` 就是 build 所在路径。

然后手动创建 [http://TutorialConfig.h.in](https://link.zhihu.com/?target=http%3A//TutorialConfig.h.in) 文件，包含以下内容：

```cmake
// the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR @PROJECT_VERSION_MAJOR@
#define Tutorial_VERSION_MINOR @PROJECT_VERSION_MINOR@
#define Tutorial_VERSION_PATCH @PROJECT_VERSION_PATCH@
```

当使用 CMake 构建项目后，会在 build 中生成一个  TutorialConfig.h 文件，内容如下：

```cmake
// the configured options and settings for Tutorial
#define Tutorial_VERSION_MAJOR 1
#define Tutorial_VERSION_MINOR 0
#define Tutorial_VERSION_PATCH 2
```

下一步在 tutorial.cpp 包含头文件 TutorialConfig.h，最后通过以下代码打印出可执行文件的名称和版本号。

```cmake
    if (argc < 2) {
      // report version
      std::cout << argv[0] << " Version " << Tutorial_VERSION_MAJOR << "."
                << Tutorial_VERSION_MINOR << std::endl;
      std::cout << "Usage: " << argv[0] << " number" << std::endl;
      return 1;
    }
```

## 指定C++标准

在 CMake 中支持特定 C++标准的最简单方法是使用 `CMAKE_CXX_STANDARD` 标准变量。在 CMakeLists.txt 中设置 `CMAKE_CXX_STANDARD` 为11，`CMAKE_CXX_STANDARD_REQUIRED` 设置为True。确保在 `add_executable` 命令之前添加 `CMAKE_CXX_STANDARD_REQUIRED` 命令。

```cmake
cmake_minimum_required(VERSION 3.15)

# set the project name and version
project(${PROJECT_NAME} VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
```

需要注意的是，如果你的gcc编译器版本够高，也可以不用指定 C++ 版本为 11。从 GCC 6.1 开始，当不指定任何版本 C++ 标准时，默认版本是 C++ 14，如果你想用 C++17 的语言，还是需要指定的。

修改完成后，需要对代码进行重新编译 `cmake --build .`，此时可以不用进行项目构建。

此时目录结构为：

```
step2/
    build/
    CMakeLists.txt
    tutorial.cpp
    TutorialConfig.h.in
```

# step 3：添加库

## 添加子目录

现在我们将向项目中添加一个库，这个库包含计算数字平方根的实现，可执行文件使用这个库，而不是编译器提供的标准平方根函数。

我们把库放在名为 MathFunctions 的子目录中。此目录包含头文件 MathFunctions.h 和源文件 mysqrt.cpp。源文件有一个名为 mysqrt 的函数，它提供了与编译器的 sqrt 函数类似的功能，MathFunctions.h 则是该函数的声明。

在 MathFunctions 目录下创建一个 CMakeLists.txt 文件，并添加以下一行：

```cmake
# MathFunctions/CMakeLists.txt
add_library(MathFunctions mysqrt.cpp)
```

表示添加一个叫 MathFunctions 的库文件。

CMake 中的 target 有可执行文件和库文件，分别使用 `add_executable` 和 `add_library` 命令生成，除了指定生成的可执行文件名/库文件名，还需要指定相关的源文件。

此时文件结构为：

```
step3/
    build/
    MathFunctions/
        CMakeLists.txt
        MathFunctions.h
        mysqrt.cpp
    CMakeLists.txt
    tutorial.cpp
    TutorialConfig.h.in
```

为了使用 MathFunctions  这个库，我们将在顶级 CMakeLists.txt 文件中添加一个 `add_subdirectory(MathFunctions)` 命令指定库所在子目录，该子目录下应包含 CMakeLists.txt 文件和代码文件。

可执行文件要使用库文件，需要能够找到库文件和对应的头文件，可以分别通过 `target_link_libraries` 和 `target_include_directories` 来指定。

使用 `target_link_libraries` 将新的库文件添加到可执行文件中，使用 `target_include_directories` 将 MathFunctions 添加为头文件目录，添加到 Tutorial 目标上，以便 mysqrt.h 可以被找到。

顶级 CMakeLists.txt 的最后几行如下所示：

```cmake
# add the MathFunctions library
add_subdirectory(MathFunctions)

# add the executable
add_executable(${PROJECT_NAME} tutorial.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC MathFunctions)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
target_include_directories(${PROJECT_NAME} PUBLIC
                           ${PROJECT_BINARY_DIR}
                           ${PROJECT_SOURCE_DIR}/MathFunctions
                           )
```

MathFunctions 库就算添加完成了，接下来就是在主函数使用该库中的函数，先在 tutorial.cpp 文件中添加头文件：

```
#include "MathFunctions.h"
```

然后使用 `mysqrt` 函数即可：

```
const double outputValue = mysqrt(inputValue);
```

## CMAKE_PREFIX_PATH

`CMAKE_PREFIX_PATH`是一个分号分隔的路径列表，用来指明软件/库安装路径前缀，以供`find_package()`，`find_program()`，`find_library()`，`find_file()`和`find_path()`命令搜索使用，这样就方便搜索可执行文件、头文件、库文件及相关配置文件等。