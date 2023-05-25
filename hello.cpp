//
// Created by Yongliang Yang on 2023/5/18.
//

#include <stdio.h>
#include <vector>

std::vector<int> func() {
    std::vector<int> vec({1,2,3});
    return std::move(vec);
}

class A {

public:
    int x;
    A() {
        x = 0;
        printf("construct %p...\n", this);
    }
    A(A& a) {
        x = 1;
        printf("copy construct...\n");
    }
    A(A&& a) {
        x = 3;
        printf("move construct %p...\n", this);
    }
    ~A() {
        printf("destruct...\n");
    }
};



A func_A() {
    int b = 1;
    printf("function stack %p\n", &b);
    A a = A();
    //return std::move(a);
    return a;
}

int main()
{
    //std::vector<int> v = func();
    int c = 1;
    printf("process stack %p\n", &c);
    A&& a = func_A();
    A b(func_A());
    //printf("%d", v[0]);

    printf("hello world!\n");
    for (int i = 0; i < 1000; ++i) {}
    printf("hello world!\n");
    printf("hello world!\n");
    printf("hello world!\n");

    printf("%p\n", &a);
    //printf("%d", v[0]);
    printf("%p\n", &b);
    return 0;
}