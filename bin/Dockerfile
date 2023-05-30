FROM ubuntu:latest
RUN mkdir /Final_AI_Server
# 需将LibTorch的依赖库和可执行文件以及编译好的文件拷贝到当前目录
COPY bin /Final_AI_Server/
WORKDIR /Final_AI_Server
ENV LD_LIBRARY_PATH /Final_AI_Server/libtorch/lib:$LD_LIBRARY_PATH
CMD ["source", "start.sh"]
EXPOSE 50051