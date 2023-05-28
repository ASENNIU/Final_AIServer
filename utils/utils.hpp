//
// Created by Yongliang Yang on 2023/5/24.
//
#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <sstream>
#include <sstream>
#include <functional>



namespace rs{
    template<typename T>
    std::vector<T> convert_string_to_vector(const std::string& str, const char delim) {
        /***
          将向量的字符串形式转为vector，向量的形式为 0.33,0,33
          */
        std::vector<T> result_vector;
        auto lastPos = str.find_first_not_of(delim, 0);
        auto pos = str.find_first_of(delim, lastPos);
        std::string str_tmp;
        T value_tmp;
        std::stringstream ss;
        while (pos != std::string::npos || lastPos != std::string::npos) {
            // 获取数据
            str_tmp = str.substr(lastPos, pos - lastPos);
            // 利用stringstream做数据转换
            ss << str_tmp;
            ss >> value_tmp;
            ss.clear();

            result_vector.emplace_back(value_tmp);

            // 寻找下一个数据
            lastPos = str.find_first_not_of(delim, pos);
            pos = str.find_first_of(delim, lastPos);
        }

        /***
         *  C++函数调用及返回的逻辑是在调用处打断点，创建一个临时匿名对象，由函数返回的局部对象为这个临时对象初始化，并用这个临时对象
         *  赋值调用方（可能有），但根据我的测试以及网上的资料，现代编译器会优化这个过程RVO，所以在这里并未做过多处理
         */   
        return result_vector;
    }

    template<typename T>
    std::string convert_verctor_string(const std::vector<T>& array, const char delim) {
        /***
          将vector转化为string，以字符delim分割
          */

        const size_t array_length = array.size();
        std::string str;
        std::stringstream ss;
        std::string tmp;

        for (size_t i = 0; i < array_length; ++i) {
            ss << array[i];
            ss >> tmp;
            ss.clear();

            str += tmp;

            if (i != array_length - 1)
                str += delim;
        }

        return str;
    }

    template<typename T>
    std::vector<int> argsort(const std::vector<T>& array, bool is_reverse = true) {
        const size_t array_length = array.size();
        std::vector<int> index_array(array_length, 0);

        for (size_t i = 0; i < array_length; ++i) {
            index_array[i] = i;
        }

        std::function<bool(int, int)> func_increase = [&array](int idx1, int idx2) {
            return array[idx1] < array[idx2];};
        std::function<bool(int, int)> func_descend = [&array](int idx1, int idx2) {
            return array[idx1] < array[idx2];};

        std::sort(index_array.begin(), index_array.end(), is_reverse ? func_descend : func_increase);

        return index_array;
    }

}


#endif