FROM centos:7
ADD protobuf-all-3.6.0.tar.gz /root/soft/
ADD cmake-3.14.0-rc2.tar.gz /root/soft/
RUN yum update -y && \
	yum install -y vim \
	wget \
	make \
	gcc \
	gcc-c++ \
	automake \
	unzip && \
	cd /root/soft/cmake-3.14.0-rc2 && \
	./configure && make && make install && \
	cd /root/soft/protobuf-3.6.0 && \
	./configure && make && make install && \
	cd /root/soft && rm -rf cmake-3.14.0-rc2 protobuf-3.6.0