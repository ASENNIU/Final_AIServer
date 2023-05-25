//
// Created by Yongliang Yang on 2023/5/25.
//
#include<cstdio>
#include<typeinfo>
#include<iostream>

void fun(const char * p) {
    printf("%s\n", p);
}
int main(int argc, const char* argv[])
{
    //printf("%s\n", typeid(argv[1]).name());
    std::cout << typeid(argv[1]).name() << '\n';
    fun(argv[1]);
    return 0;
}