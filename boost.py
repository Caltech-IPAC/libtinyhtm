#! /usr/bin/env python
# encoding: utf-8

def configure(conf):
    def get_param(varname,default):
        return getattr(Options.options,varname,'')or default

    # Find Boost
    if conf.options.boost_dir:
        if not conf.options.boost_incdir:
            conf.options.boost_incdir=conf.options.boost_dir + "/include"
        if not conf.options.boost_libdir:
            conf.options.boost_libdir=conf.options.boost_dir + "/lib"

    if conf.options.boost_incdir:
        boost_incdir=[conf.options.boost_incdir]
    else:
        boost_incdir=[]
    if conf.options.boost_libdir:
        boost_libdir=[conf.options.boost_libdir]
    else:
        boost_libdir=[]

    if conf.options.boost_libs:
        boost_libs=conf.options.boost_libs.split()
    else:
        boost_libs=['boost_system','boost_filesystem','boost_iostreams']

    conf.check_cxx(msg="Checking for Boost",
                  header_name='boost/filesystem.hpp',
                  includes=boost_incdir,
                  uselib_store='boost',
                  libpath=boost_libdir,
                  rpath=boost_libdir,
                  lib=boost_libs)

def options(opt):
    boost=opt.add_option_group('Boost Options')
    boost.add_option('--boost-dir',
                   help='Base directory where boost is installed')
    boost.add_option('--boost-incdir',
                   help='Directory where boost include files are installed')
    boost.add_option('--boost-libdir',
                   help='Directory where boost library files are installed')
    boost.add_option('--boost-libs',
                   help='Names of the boost libraries without prefix or suffix\n'
                   '(e.g. "boost_system boost_filesystem")')
