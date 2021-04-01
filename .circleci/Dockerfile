FROM centos:7

RUN set -e; \
	curl -s https://packagecloud.io/install/repositories/varnishcache/varnish66/script.rpm.sh | bash ; \
	yum install -y epel-release; \
	yum install -y \
	    git \
	    varnish-devel \
	    make \
	    automake \
	    libtool \
	    python-sphinx
