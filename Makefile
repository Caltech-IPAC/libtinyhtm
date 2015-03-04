WAF=python waf
CONFIGURE=sh configure

all: configure build

configure:
	@$(WAF) configure --ccfits-dir=$(CM_TPS_DIR) --hdf5-dir=$(CM_TPS_DIR) --boost-dir=${CM_ENV_DIR}/misc --prefix=${PWD}

build:
	@$(WAF) build

clean:
	@$(WAF) clean

distclean:
	rm -rf build

list:
	@$(WAF) list

docs:
	@$(WAF) docs

install:
	@$(WAF) install

installdocs:
	@$(WAF) install

uninstall:
	@$(WAF) uninstall

test:
	@$(WAF) test

dist:
	@$(WAF) dist


.PHONY: all configure build clean dist distclean list docs install uninstall test dist

