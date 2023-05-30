FROM ubuntu:latest
RUN mkdir /Final_AIServer
# 需将LibTorch的依赖库和可执行文件以及编译好的文件拷贝到当前目录
COPY . /Final_AIServer/
WORKDIR /Final_AIServer
ENV LD_LIBRARY_PATH /Final_AIServer/libtorch/lib:$LD_LIBRARY_PATH
CMD /Final_AIServer/Final_AISever /Final_AIServer/RS_CO_MODEL.pt
EXPOSE 50051