//
// Created by Yongliang Yang on 2023/5/24.
//
#include "utils.hpp"
#include <vector>
#include <sstream>


#ifndef UTILS_H
#define UTILS_H
namespace rs{
    template<typename T>
    vector<T>& convert_string_to_vector(const std::string& str, const std::string& delim) {
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
            tmp = str.substr(lastPos, pos - lastPos);
            // 利用stringstream做数据转换
            ss << tmp;
            ss >> value_tmp;
            ss.clear();

            result_vector.emplace_back(value_tmp);

            // 寻找下一个数据
            lastPos = str.find_first_not_of(delim, pos);
            pos = str.find_first_of(delim, lastPos);
        }
        return result_vector;
    }

}


#endif