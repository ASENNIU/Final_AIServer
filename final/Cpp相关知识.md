# 使用std::function封装函数

```c++
std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...)
```

在设计回调函数的时候，无可避免地会接触到可回调对象。在C++11中，提供了`std::function`和`std::bind`两个方法来对可回调对象进行统一和封装。

**可调用对象**

C++中有如下几种可调用对象：**函数、函数指针、lambda表达式、bind对象、函数对象**。其中，[lambda表达式](https://so.csdn.net/so/search?q=lambda%E8%A1%A8%E8%BE%BE%E5%BC%8F&spm=1001.2101.3001.7020)和bind对象是C++11标准中提出的(bind机制并不是新标准中首次提出，而是对旧版本中bind1st和bind2st的合并)。个人认为五种可调用对象中，函数和函数指针本质相同，而lambda表达式、bind对象及函数对象则异曲同工。

## 函数

这里的函数指的是普通函数，没什么可拓展的。

## 函数指针

插播一下函数指针和函数类型的区别：

- 函数指针指向的是函数而非对象。和其他指针类型一样，函数指针指向某种特定类型；

- **函数类型由它的返回值和参数类型决定，与函数名无关**。
   例如：

  ```c++
  bool fun(int a, int b)
  ```

  上述函数的函数类型是：`bool(int, int)`

  上述函数的函数指针pf是：`bool (*pf)(int, int)`

  一般对于函数来说，**函数名即为函数指针**：

  ```c++
  # include <iostream>

  int fun(int x, int y) {                         //被调用的函数
      std::cout << x + y << std::endl;
  	return x + y;
  }

  int fun1(int (*fp)(int, int), int x, int y) {   //形参为函数指针
  	return fp(x, y);
  }

  typedef int (*Ftype)(int, int);                 //定义一个函数指针类型Ftype
  int fun2(Ftype fp, int x, int y) { 
  	return fp(x, y);
  }

  int main(){
  	fun1(fun, 100, 100);                          //函数fun1调用函数fun
  	fun2(fun, 200, 200);                          //函数fun2调用函数fun
  }

  ```

  可以看出，**函数指针作为参数，可以在调用函数中调用函数指针代表的函数内容**。

  lambda表达式

  lambda表达式就是一段可调用的代码。主要适合于只用到一两次的简短代码段。**由于lambda是匿名的，所以保证了其不会被不安全的访问**:

  ```c++
  # include <iostream>

  int fun3(int x, int y){
  	auto f = [](int x, int y) { return x + y; };  //创建lambda表达式,如果参数列表为空，可以省去() 
  	std::cout << f(x, y) << std::endl;            //调用lambda表达式
  }

  int main(){
      fun3(300, 300);
  }

  ```

## bind对象

std::bind可以用来生产，一个可调用对象来适应原对象的参数列表。具体的内容会在下文讲解。

## 函数对象

**重载了函数调用运算符()的类的对象，即为函数对象**。

## std::function

由上文可以看出：由于可调用对象的定义方式比较多，但是函数的调用方式较为类似，因此需要使用一个统一的方式保存可调用对象或者传递可调用对象。于是，`std::function`就诞生了。

**std::function是一个可调用对象包装器，是一个类模板，可以容纳除了类成员函数指针之外的所有可调用对象，它可以用统一的方式处理函数、函数对象、函数指针，并允许保存和延迟它们的执行**。

定义function的一般形式：

```c++
# include <functional>
std::function<函数类型>
```

例如：

```c++
# include <iostream>
# include <functional>

typedef std::function<int(int, int)> comfun;

// 普通函数
int add(int a, int b) { return a + b; }

// lambda表达式
auto mod = [](int a, int b){ return a % b; };

// 函数对象类
struct divide{
    int operator()(int denominator, int divisor){
        return denominator/divisor;
    }
};

int main(){
	comfun a = add;
	comfun b = mod;
	comfun c = divide();
    std::cout << a(5, 3) << std::endl;
    std::cout << b(5, 3) << std::endl;
    std::cout << c(5, 3) << std::endl;
}

```

std::function可以取代函数指针的作用，因为它可以延迟函数的执行，特别适合作为`回调函数`使用。它比普通函数指针更加的灵活和便利。

故而，std::function的作用可以归结于：

1. **std::function对C++中各种可调用实体(普通函数、Lambda表达式、函数指针、以及其它函数对象等)的封装，形成一个新的可调用的std::function对象，简化调用**；
2. **std::function对象是对C++中现有的可调用实体的一种类型安全的包裹(如：函数指针这类可调用实体，是类型不安全的)**。

## std::bind

std::bind可以看作一个通用的函数适配器，**它接受一个可调用对象，生成一个新的可调用对象来适应原对象的参数列表**。

std::bind将可调用对象与其参数一起进行绑定，绑定后的结果可以使用std::function保存。std::bind主要有以下两个作用：

- **将可调用对象和其参数绑定成一个仿函数**；
- **只绑定部分参数，减少可调用对象传入的参数**。

调用bind的一般形式：

```c++
auto newCallable = bind(callable, arg_list);
```

该形式表达的意思是：当调用`newCallable`时，会调用`callable`，并传给它`arg_list`中的参数。

需要注意的是：arg_list中的参数可能包含形如_n的名字。其中n是一个整数，这些参数是占位符，表示newCallable的参数，它们占据了传递给newCallable的参数的位置。数值n表示生成的可调用对象中参数的位置。

直接文字可能不那么生动，不如看代码：

```c++
#include <iostream>
#include <functional>

class A {
public:
    void fun_3(int k,int m) {
        std::cout << "print: k = "<< k << ", m = " << m << std::endl;
    }
};

void fun_1(int x,int y,int z) {
    std::cout << "print: x = " << x << ", y = " << y << ", z = " << z << std::endl;
}

void fun_2(int &a,int &b) {
    ++a;
    ++b;
    std::cout << "print: a = " << a << ", b = " << b << std::endl;
}

int main(int argc, char * argv[]) {
    //f1的类型为 function<void(int, int, int)>
    auto f1 = std::bind(fun_1, 1, 2, 3); 					//表示绑定函数 fun 的第一，二，三个参数值为： 1 2 3
    f1(); 													//print: x=1,y=2,z=3

    auto f2 = std::bind(fun_1, std::placeholders::_1, std::placeholders::_2, 3);
    //表示绑定函数 fun 的第三个参数为 3，而fun 的第一，二个参数分别由调用 f2 的第一，二个参数指定
    f2(1, 2);												//print: x=1,y=2,z=3
 
    auto f3 = std::bind(fun_1, std::placeholders::_2, std::placeholders::_1, 3);
    //表示绑定函数 fun 的第三个参数为 3，而fun 的第一，二个参数分别由调用 f3 的第二，一个参数指定
    //注意： f2  和  f3 的区别。
    f3(1, 2);												//print: x=2,y=1,z=3

    int m = 2;
    int n = 3;
    auto f4 = std::bind(fun_2, std::placeholders::_1, n); //表示绑定fun_2的第一个参数为n, fun_2的第二个参数由调用f4的第一个参数（_1）指定。
    f4(m); 													//print: a=3,b=4
    std::cout << "m = " << m << std::endl;					//m=3  说明：bind对于不事先绑定的参数，通过std::placeholders传递的参数是通过引用传递的,如m
    std::cout << "n = " << n << std::endl;					//n=3  说明：bind对于预先绑定的函数参数是通过值传递的，如n
    
    A a;
    //f5的类型为 function<void(int, int)>
    auto f5 = std::bind(&A::fun_3, &a, std::placeholders::_1, std::placeholders::_2); //使用auto关键字
    f5(10, 20);												//调用a.fun_3(10,20),print: k=10,m=20

    std::function<void(int,int)> fc = std::bind(&A::fun_3, a,std::placeholders::_1,std::placeholders::_2);
    fc(10, 20);   											//调用a.fun_3(10,20) print: k=10,m=20 

    return 0; 
}
```

由此例子可以看出：

- **预绑定的参数是以值传递的形式，不预绑定的参数要用std::placeholders(占位符)的形式占位，从_1开始，依次递增，是以引用传递的形式**；
- **std::placeholders表示新的可调用对象的第几个参数，而且与原函数的该占位符所在位置的进行匹配**；
- **bind绑定类成员函数时，第一个参数表示对象的成员函数的指针，第二个参数表示对象的地址，这是因为对象的成员函数需要有this指针**。并且编译器不会将对象的成员函数隐式转换成函数指针，需要通过&手动转换；
- **std::bind的返回值是可调用实体，可以直接赋给std::function**


# std::future

在本次线程池中的实例：

```c++
auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
```

我们想要从线程中返回异步任务结果，一般需要依靠**全局变量**；从安全角度看，有些不妥；为此C++11提供了std::future类模板，future对象提供访问异步操作结果的机制，很轻松解决从异步任务中返回结果。

参考博客：https://blog.csdn.net/c_base_jin/article/details/89761718

唯一期望(unique futures，std::future<>) std::future的实例只能与一个指定事件相关联。

```c++
//std::future类模板定义头文件<future>的函数声明
template< class T > 
class future;
//数据有关的期望
template< class T > 
class future<T&>;
//数据无关的期望
template<>          
class future<void>;

```

## 对于future的说明

- std::async 、 **std::packaged_task** 或 std::promise 能提供一个std::future对象给该异步操作的创建者
- 异步操作的创建者能用各种方法查询、等待或从 std::future 提取值。若异步操作仍未提供值，则这些方法可能阻塞。
- 异步操作准备好发送结果给创建者时，它能通过接口（eg,std::promise::set_value std::future） 修改共享状态的值。

![](E:/Workspace/Final_AIServer/final/assert/future%E7%9A%84%E6%88%90%E5%91%98%E5%87%BD%E6%95%B0.png)

**细节说明：**

- wait() 操作

  ```c++
  void wait() const;
  ```

  当共享状态值是不可以用时，调用wait接口可以一直阻塞，直到共享状态变为"就绪"时，就变为可以用了。

- get操作。get是获取共享状态的结果它有以下三种形式：

  ```c++
  //仅为泛型 future 模板的成员
  T get();
  //(仅为 future<T&> 模板特化的成员)
  T& get();
  //仅为 future<void> 模板特化的成员
  void get();
  ```

  如果我们没有调用wait接口，而是直接掉用get接口，它等价于先调用wait()而后在调用get接口，得到异步操作的结果。

  **当调用此方法后 valid() 为 false ，共享状态被释放，即future对象释一次性的事件。**

  按照自己的理解，将std::future对象的使用以及内部逻辑用时序图进行表达，如下：

  ![](E:/Workspace/Final_AIServer/final/assert/future%E5%AF%B9%E8%B1%A1%E6%97%B6%E5%BA%8F%E5%9B%BE.png)

  ​

  **std::future使用**

  下面就用std::future对象来获取异步操作的结果，**没有使用到全局变量，逻辑非常清晰**，代码如下：

  ```c++
  //通过async来获取异步操作结果
  std::future<int>  result = std::async([](){ 
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
      return 8; 
  });

  std::cout << "the future result : " << result.get() << std::endl;
  std::cout << "the future status : " << result.valid() << std::endl;
  try
  {
      result.wait();  //或者 result.get() ,会异常
    //因此std::future只能用于单线程中调用 ，多线程调用使用std::share_future();
  }
  catch (...)
  {
      std::cout << "get error....\n ";
  }

  ```

  程序输出：

  ```plain text
  the future result : 8
  the future state  : 0
  get error....
  ```

## 在本次线程池中的使用

结合起来看，函数签名使用了为尾型推断的技巧，使用decltype(f(args..))获取任务函数的返回类型，同时封装成一个std::future对象

```c++
auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
```



# std::packaged_task

std::packaged_task 包装一个可调用的对象，并且允许异步获取该可调用对象产生的结果，也就是将其的返回值传给future。它和promise类似，关于promise和future，可以参考：[c++多线程实践-promise+future - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/448035015?)

区别在于：（参考：[深入浅出 c++11 std::async - 程远春 - 博客园 (cnblogs.com)](https://link.zhihu.com/?target=https%3A//www.cnblogs.com/chengyuanchun/p/5394843.html)）

> td::packaged_task包装的是一个异步操作（函数或lambda表达式），而std::promise包装的是一个值，都是为了方便异步操作的，因为有时我需要获 取线程中的某个值，这时就用std::promise，而有时我需要获一个异步操作的返回值，这时就用std::packaged_task。当然也可以将一个异步操作的结果保存到std::promise中

std::packaged_task<函数返回类型(参数类型)> 变量名{函数名}

一个获取异步操作结果的小例子：

```c++
#include<iostream>
#include<future>
#include<thread>

int TripleX_func(int x){
	x = x*3;
	std::cout<<"3X thread id:"<<std::endl;
	std::cout<<std::this_thread::get_id()<<std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(5));
	return x; 
} 
int main(){
	// 将函数（某种操作后的值）打包起来
	// std::packaged_task<函数返回类型(参数类型)> 变量名(函数名)
	std::packaged_task<int(int)> pt{TripleX_func};
	//并将结果返回给future，类型是int 
	std::future<int> fu = pt.get_future();
	//future提供了一些函数比如get(),wait(),wait_for()。
	//一般用get()来获取future所得到的结果
	//如果异步操作还没有结束，那么会在此等待异步操作的结束，并获取返回的结果。
	std::thread t(std::ref(pt), 5);
	std::cout<<fu.get()<<std::endl;
	//输出3X线程和main线程的id，可以发现是两个不同的ID。 
	std::cout<<"main thread id:"<<std::endl;
	std::cout<<std::this_thread::get_id()<<std::endl;
	t.join();
	return 0;
	
}
```

std::packaged_task 对象内部包含了两个最基本元素，一、被包装的任务(stored task)，任务(task)是一个可调用的对象，如函数指针、成员函数指针或者函数对象；二、共享状态(shared state)，可以通过 std::packged_task::get_future() 来获取与共享状态相关联的 std::future 对象，在调用该函数之后，两个对象共享相同的共享状态。

调用packaged_task对象包装的任务（操作：函数/labda表达式）一般会发生两种情况：

- 如果成功调用 packaged_task 所包装的任务，则返回值（如果被包装的对象有返回值的话）被保存在 packaged_task 的共享状态中。
- 如果调用 packaged_task 所包装的任务失败，并且抛出了异常，则异常也会被保存在 packaged_task 的共享状态中。

以上两种情况都使共享状态的标志变为 ready。