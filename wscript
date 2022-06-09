#! /usr/bin/env python
# encoding: utf-8
#
# Copyright (C) 2011 Serge Monkewitz IPAC/Caltech
#

from __future__ import with_statement
import os
import sys
import traceback

from waflib import Build, Logs, Utils

APPNAME = 'tinyhtm'
VERSION = '0.5'

top = '.'
out = 'build'


def options(ctx):
    ctx.load('compiler_c compiler_cxx cxx14 hdf5_cxx boost gnu_dirs')
    ctx.add_option('--debug', help='Include debug symbols and turn ' +
                                   'compiler optimizations off',
                   action='store_true', default=False, dest='debug')
    ctx.add_option('--enable-gcov',
                   help='Enable code coverage analysis. This option ' +
                        'requires the use of GCC and implies --debug!',
                   action='store_true', default=False, dest='enable_gcov')
    ctx.add_option('--enable-valgrind',
                   help='Run unit tests under valgrind. WARNING: this ' +
                        'substantially increases unit test run-time!',
                   action='store_true', default=False, dest='enable_valgrind')

def configure(ctx):
    ctx.env.TINYHTM_VERSION = VERSION
    ctx.load('compiler_c compiler_cxx cxx14 hdf5_cxx boost gnu_dirs')
    ctx.check_boost('filesystem system iostreams')
    
    ctx.env.append_value('CXXFLAGS', '-Wall')
    ctx.env.append_value('CXXFLAGS', '-Wextra')
    ctx.env.append_value('CXXFLAGS', '-D__STDC_CONSTANT_MACROS')

    ctx.env.append_value('CFLAGS', '-Wall')
    ctx.env.append_value('CFLAGS', '-Wextra')
    ctx.env.append_value('CFLAGS', '-std=c99')
    ctx.env.append_value('CFLAGS', '-D_GNU_SOURCE')

    # Test for __attribute__ support
    ctx.check_cc(fragment='''void foo(int x __attribute__ ((unused))) { }
                             __attribute__ ((unused)) void bar() { }
                             int main() { return 0; }''',
                 define_name='HAVE_ATTRIBUTE_UNUSED',
                 msg='Checking for __attribute__ ((unused))')
    ctx.check_cc(fragment='''typedef struct { double a; double b; } test __attribute__ ((aligned(16)));
                             int main() { return 0; }''',
                 define_name='HAVE_ATTRIBUTE_ALIGNED',
                 msg='Checking for __attribute__ ((aligned()))')

    # Check for libm and libpthread
    ctx.check_cc(lib='m', uselib_store='M')
    ctx.check_cc(lib='pthread', uselib_store='PTHREAD') 

    # Undefine FAST_ALLOC (or define it as 0) to allocate nodes with
    # malloc() instead. Useful when checking memory safety, e.g. with
    # valgrind.
    ctx.define('FAST_ALLOC',1)
    ctx.write_config_header('include/tinyhtm/config.h')

    # Massage CFLAGS depending on whether or not
    # code coverage / debugging is requestd

    ctx.env.append_value('CFLAGS', '-g')
    ctx.env.append_value('CXXFLAGS', '-g')

    if ctx.options.enable_gcov:
        ctx.find_program('gcov', var='GCOV', mandatory=False)
        if ctx.env['GCOV']:
            ctx.env.append_value('CFLAGS', '-fprofile-arcs')
            ctx.env.append_value('CFLAGS', '-ftest-coverage')
            ctx.env.append_value('CXXFLAGS', '-fprofile-arcs')
            ctx.env.append_value('CXXFLAGS', '-ftest-coverage')
            ctx.env.append_value('LINKFLAGS', '-fprofile-arcs')
    elif not ctx.options.debug:
        ctx.env.append_value('CFLAGS', '-O3')
        ctx.env.append_value('CFLAGS', '-mtune=native')
        ctx.env.append_value('CFLAGS', '-march=native')
        ctx.env.append_value('CFLAGS', '-DNDEBUG')
        ctx.env.append_value('CXXFLAGS', '-O3')
        ctx.env.append_value('CXXFLAGS', '-fopenmp')
        ctx.env.append_value('CXXFLAGS', '-mtune=native')
        ctx.env.append_value('CXXFLAGS', '-march=native')
        ctx.env.append_value('CXXFLAGS', '-DNDEBUG')

    # Automatic parallelization of some STL libraries.  Does not seem
    # to help.
    ctx.env.append_value('CXXFLAGS', '-D_GLIBCXX_PARALLEL')
    ctx.env.append_value('CXXFLAGS', '-fopenmp')
    ctx.env.append_value('LINKFLAGS', '-fopenmp')

    # locate doxygen
    ctx.find_program('doxygen', var='DOXYGEN', mandatory=False)
    if ctx.options.enable_valgrind:
        ctx.find_program('valgrind', var='VALGRIND', mandatory=False)


def build(ctx):
    # C interface
    # static library
    c_sources=['src/common.cxx',
               'src/select.cxx',
               'src/geometry.cxx',
               'src/htm.cxx',
               'src/htm/_htm_ids_init.cxx',
               'src/htm/_htm_s2circle_htmcov.cxx',
               'src/htm/htm_s2circle_ids.cxx',
               'src/htm/_htm_s2cpoly_htmcov/_htm_isect_test.cxx',
               'src/htm/_htm_s2cpoly_htmcov/_htm_s2cpoly_htmcov.cxx',
               'src/htm/htm_s2cpoly_ids.cxx',
               'src/htm/_htm_s2ellipse_htmcov/_htm_s2ellipse_htmcov.cxx',
               'src/htm/_htm_s2ellipse_htmcov/_htm_s2ellipse_isect.cxx',
               'src/htm/htm_s2ellipse_ids.cxx',
               'src/htm/_htm_simplify_ids.cxx',
               'src/htm/_htm_subdivide.cxx',
               'src/htm/htm_tree_s2circle.cxx',
               'src/htm/htm_tree_s2circle_range.cxx',
               'src/htm/htm_tree_s2circle_scan.cxx',
               'src/htm/htm_tree_s2cpoly.cxx',
               'src/htm/htm_tree_s2cpoly_scan.cxx',
               'src/htm/htm_tree_s2cpoly_range.cxx',
               'src/htm/htm_tree_s2ellipse.cxx',
               'src/htm/htm_tree_s2ellipse_scan.cxx',
               'src/htm/htm_tree_s2ellipse_range.cxx',
               'src/htm/htm_v3_id.cxx',
               'src/htm/htm_v3p_idsort/_htm_path_sort/_htm_partition.cxx',
               'src/htm/htm_v3p_idsort/_htm_path_sort/_htm_path_sort.cxx',
               'src/htm/htm_v3p_idsort/_htm_rootsort/_htm_rootpart.cxx',
               'src/htm/htm_v3p_idsort/_htm_rootsort/_htm_rootsort.cxx',
               'src/htm/htm_v3p_idsort/htm_v3p_idsort.cxx',
               'src/tree.cxx']
    ctx.stlib(
        source=c_sources,
        includes='src include/tinyhtm',
        target='tinyhtm',
        name='tinyhtm_st',
        install_path=ctx.env.LIBDIR,
        use='cxx14 M hdf5 hdf5_cxx'
    )
    # shared library (required by cgo)
    ctx.shlib(
        source=c_sources,
        includes='src include/tinyhtm',
        target='tinyhtm',
        name='tinyhtm_sh',
        install_path=ctx.env.LIBDIR,
        use='cxx14 M hdf5 hdf5_cxx'
    )

    # C++ interface
    # static library
    cxx_sources=\
        ['src/Cartesian.cxx',
         'src/Spherical.cxx',
         'src/Query/Query.cxx',
         'src/sort_and_index/tree_compress/tree_compress.cxx',
         'src/sort_and_index/tree_compress/hash_table/hash_table_get.cxx',
         'src/sort_and_index/tree_compress/hash_table/hash_table_grow.cxx',
         'src/sort_and_index/tree_compress/hash_table/hash_table_destroy.cxx',
         'src/sort_and_index/tree_compress/hash_table/hash_table_add.cxx',
         'src/sort_and_index/tree_compress/hash_table/hash_table_init.cxx',
         'src/sort_and_index/tree_compress/write_tree_header.cxx',
         'src/sort_and_index/tree_compress/compress_node.cxx',
         'src/sort_and_index/reverse_file.cxx',
         'src/sort_and_index/now.cxx',
         'src/sort_and_index/append_htm.cxx',
         'src/sort_and_index/blk_writer/blk_write.cxx',
         'src/sort_and_index/ext_sort/mrg_npasses.cxx',
         'src/sort_and_index/tree_gen/layout_node.cxx',
         'src/sort_and_index/tree_gen/assign_block.cxx',
         'src/sort_and_index/tree_gen/emit_node.cxx',
         'src/sort_and_index/tree_gen/estimate_node_size.cxx',
         'src/sort_and_index/tree_gen/finish_root.cxx',
         'src/sort_and_index/tree_gen/add_node.cxx']

    ctx.stlib(
        source=cxx_sources,
        includes='src include/tinyhtm',
        target='tinyhtmcxx',
        name='tinyhtmcxx_st',
        install_path=ctx.env.LIBDIR,
        use='cxx14 M hdf5 hdf5_cxx tinyhtm BOOST'
    )
    # shared library
    ctx.shlib(
        source=cxx_sources,
        includes='src include/tinyhtm',
        target='tinyhtmcxx',
        name='tinyhtmcxx_sh',
        install_path=ctx.env.LIBDIR,
        use='cxx14 M hdf5 hdf5_cxx tinyhtm BOOST'
    )

    # tree index generator
    ctx.program(
        source=[
            'src/htm_tree_gen/usage.cxx',
            'src/htm_tree_gen/htm_tree_gen.cxx',
            'src/htm_tree_gen/blk_sort_ascii/eat_delim.cxx',
            'src/htm_tree_gen/blk_sort_ascii/eat_ws.cxx',
            'src/htm_tree_gen/blk_sort_ascii/blk_sort_ascii.cxx',
            'src/tree_entry.cxx'],
        includes='src include/tinyhtm',
        target='htm_tree_gen',
        name='htm_tree_gen',
        install_path=ctx.env.BINDIR,
       use='cxx14 M PTHREAD tinyhtm_st tinyhtmcxx_st hdf5_cxx BOOST'
   )
    # Convert old format to hdf5
    ctx.program(
        source=['src/htm_convert_to_hdf5.cxx',
                'src/tree_entry.cxx'],
        includes='src include/tinyhtm',
        target='htm_convert_to_hdf5',
        name='htm_convert_to_hdf5',
        install_path=ctx.env.BINDIR,
        use='cxx14 M PTHREAD tinyhtm_st hdf5_cxx tinyhtmcxx_st BOOST'
    )

    # point counting utility
    ctx.program(
        source='src/tree_count.cxx',
        includes='src include/tinyhtm',
        target='htm_tree_count',
        name='htm_tree_count',
        install_path=ctx.env.BINDIR,
        use='cxx14 M tinyhtm_st hdf5_cxx'
    )
    # id listing utility
    ctx.program(
        source='src/id_list.cxx',
        includes='src include/tinyhtm',
        target='htm_id_list',
        name='htm_id_list',
        install_path=ctx.env.BINDIR,
        use='cxx14 M tinyhtm_st'
    )
    # test cases
    ctx.objects(source='test/rand.cxx test/cmp.cxx',
                includes='src include/tinyhtm',
                target='testobjs')
    for t in ('htm', 'geometry', 'select', 'ranges'):
        ctx.program(
            source='test/test_%s.cxx' % t,
            includes='src include/tinyhtm',
            target='test/test_' + t,
            install_path=False,
            use='cxx14 testobjs M tinyhtm_st tinyhtmcxx_st'
        )

    # install headers
    # one file to the top INCLUDEDIR...
    ctx.install_files(ctx.env.INCLUDEDIR, ['src/tinyhtm.h'])

    #...and the rest a level below.
    ctx.install_files(ctx.env.INCLUDEDIR + '/tinyhtm',
                      ctx.path.ant_glob('src/tinyhtm/*')
                      + ctx.path.ant_glob('**/config.h'))
    ctx.install_files(ctx.env.INCLUDEDIR + '/tinyhtm',
                      ctx.path.ant_glob('src/**/*.hxx'),
                      cwd=ctx.path.find_dir('src'), relative_trick=True)

    # install documentation
    docs_dir = ctx.path.find_dir('docs/html')
    if docs_dir != None:
        ctx.install_files('${PREFIX}/docs/lib/tinyhtm', docs_dir.ant_glob('*'))


class TestContext(Build.BuildContext):
    cmd = 'test'
    fun = 'test'

class Tests(object):
    def __init__(self):
        self.unit_tests = []

    def utest(self, **kw):
        nodes = kw.get('source', [])
        if not isinstance(nodes, list):
            self.unit_tests.append(nodes)
        else:
            self.unit_tests.extend(nodes)

    def run(self, ctx):
        nok, nfail, nexcept = (0, 0, 0)
        if not ctx.env['VALGRIND']:
            Logs.pprint('CYAN', '\nconfigure did not find valgrind, or was not run with ' +
                        '--enable-valgrind: running unit tests without memory checks.\n')
        for utest in self.unit_tests:
            msg = 'Running %s' % utest
            msg += ' ' * max(0, 40 - len(msg))
            Logs.pprint('CYAN', msg, sep=': ')
            out = utest.change_ext('.log')
            if ctx.env['VALGRIND']:
                args = [ctx.env['VALGRIND'],
                        '--leak-check=full',
                        '--error-exitcode=1',
                        utest.abspath()
                       ]
            else:
                args = [utest.abspath()]
            with open(out.abspath(), 'wb') as f:
                try:
                    proc = Utils.subprocess.Popen(args, shell=False,
                                                  env=ctx.env.env or None,
                                                  stderr=f, stdout=f)
                    proc.communicate()
                except:
                    nexcept += 1
                    ex_type, ex_val, _ = sys.exc_info()
                    msg = traceback.format_exception_only(ex_type, ex_val)[-1].strip()
                    Logs.pprint('RED', msg)
                else:
                    if proc.returncode != 0:
                        nfail += 1
                        Logs.pprint('YELLOW', 'FAIL [see %s]' % out.abspath())
                    else:
                        nok += 1
                        Logs.pprint('CYAN', 'OK')
        if nfail == 0 and nexcept == 0:
            Logs.pprint('CYAN', '\nAll %d tests passed!\n' % nok)
        else:
            Logs.pprint('YELLOW', '\n%d tests passed, %d failed, and %d failed to run\n' %
                        (nok, nfail, nexcept))
            ctx.fatal('One or more tinyhtm unit tests failed')

def test(ctx):
    tests = Tests()
    tests.utest(source=ctx.path.get_bld().make_node('test/test_select'))
    tests.utest(source=ctx.path.get_bld().make_node('test/test_geometry'))
    tests.utest(source=ctx.path.get_bld().make_node('test/test_htm'))
    tests.utest(source=ctx.path.get_bld().make_node('test/test_ranges'))
    tests.run(ctx)
    if not ctx.env['GCOV']:
        Logs.pprint('CYAN', 'configure did not find gcov or was not run with ' +
                    '--enable-gcov: skipping code coverage reports!\n')
    else:
        # TODO: this is an egregious hack
        for s in ('select.cxx',
                  'geometry.cxx',
                  'htm.cxx',
                  'htm/_htm_ids_init.cxx',
                  'htm/_htm_s2circle_htmcov.cxx',
                  'htm/htm_s2circle_ids.cxx',
                  'htm/_htm_s2cpoly_htmcov/_htm_isect_test.cxx',
                  'htm/_htm_s2cpoly_htmcov/_htm_s2cpoly_htmcov.cxx',
                  'htm/htm_s2cpoly_ids.cxx',
                  'htm/_htm_s2ellipse_htmcov/_htm_s2ellipse_htmcov.cxx',
                  'htm/_htm_s2ellipse_htmcov/_htm_s2ellipse_isect.cxx',
                  'htm/htm_s2ellipse_ids.cxx',
                  'htm/_htm_simplify_ids.cxx',
                  'htm/_htm_subdivide.cxx',
                  'htm/htm_tree_s2circle.cxx',
                  'htm/htm_tree_s2circle_range.cxx',
                  'htm/htm_tree_s2circle_scan.cxx',
                  'htm/htm_tree_s2cpoly.cxx',
                  'htm/htm_tree_s2cpoly_scan.cxx',
                  'htm/htm_tree_s2cpoly_range.cxx',
                  'htm/htm_tree_s2ellipse.cxx',
                  'htm/htm_tree_s2ellipse_scan.cxx',
                  'htm/htm_tree_s2ellipse_range.cxx',
                  'htm/htm_v3_id.cxx',
                  'htm/htm_v3p_idsort/_htm_path_sort/_htm_partition.cxx',
                  'htm/htm_v3p_idsort/_htm_path_sort/_htm_path_sort.cxx',
                  'htm/htm_v3p_idsort/_htm_rootsort/_htm_rootpart.cxx',
                  'htm/htm_v3p_idsort/_htm_rootsort/_htm_rootsort.cxx',
                  'htm/htm_v3p_idsort/htm_v3p_idsort.cxx'):
            ctx(rule='${GCOV} --object-file ${TGT} ${SRC}',
                source='src/' + s,
                target='src/' + s + '.1.gcno',
                always=True,
                shell=False)


class DocsContext(Build.BuildContext):
    cmd = 'docs'
    fun = 'docs'

    def __init__(self, **kw):
        super(DocsContext, self).__init__(**kw)
        self.out_dir = self.top_dir
        self.bldnode = self.root.find_dir(self.top_dir)

def docs(ctx):
    if not ctx.env['DOXYGEN']:
        ctx.fatal('configure did not find doxygen - API docs cannot be built.')
    ctx(rule='rm -rf docs; ${DOXYGEN} ${SRC}',
        source='doxygen.conf',
        always=True,
        shell=True)

