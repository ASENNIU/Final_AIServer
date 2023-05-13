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

![](./assert/future%E7%9A%84%E6%88%90%E5%91%98%E5%87%BD%E6%95%B0.png)

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

  ![](./assert/future%E5%AF%B9%E8%B1%A1%E6%97%B6%E5%BA%8F%E5%9B%BE.png)

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