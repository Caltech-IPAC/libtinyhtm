WAF=python waf
CONFIGURE=sh configure

all: distclean configure build

configure:
	@$(CONFIGURE) --hdf5-dir=/home/boo/ipac/src/hdf-bin

build:
	@$(WAF) build

clean:
	@$(WAF) clean

distclean:
	@$(WAF) distclean

list:
	@$(WAF) list

docs:
	@$(WAF) docs

install:
	@$(WAF) install

uninstall:
	@$(WAF) uninstall

test:
	@$(WAF) test

dist:
	@$(WAF) dist


.PHONY: all configure build clean dist distclean list docs install uninstall test dist

