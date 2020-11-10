FROM centos:7

# install Varnish 6.5 from https://packagecloud.io/varnishcache
RUN curl -s https://packagecloud.io/install/repositories/varnishcache/varnish65/script.rpm.sh | bash
# the epel repo contains jemalloc
RUN yum install -y epel-release
# install our dependencies
RUN yum install -y git make automake libtool python-sphinx varnish-devel
# download the top of the varnish-modules 6.5 branch
RUN git clone --branch 6.5 --single-branch https://github.com/varnish/varnish-modules.git
# jump into the directory
WORKDIR /varnish-modules
# prepare the build, build, check and install
RUN ./bootstrap && \
    ./configure && \
    make && \
    make check -j 4 && \
    make install
