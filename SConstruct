# Emacs, this is -*-python-*-
import os, os.path
import urllib

def fetch(target, source, env):
    abspath = target[0].get_abspath()
    vers, fname = abspath.rsplit('/', 2)[-2:]
    url = 'http://www.unicode.org/Public/%s/ucd/%s' % (vers, fname)
    print 'Fetching %s' % url
    urllib.urlretrieve(url, abspath)

fetch_bld = Builder(action=fetch)

env = Environment(BUILDERS = {'Fetch': fetch_bld})

# Compiler options

env['CCFLAGS'] = '-g -W -Wall -Wno-multichar'
env['CXXFLAGS'] = '-g -W -Wall -Wno-multichar --std=c++14'
env['LIBPATH'] = ['lib']
env['CPPPATH'] = ['include']

env.ParseConfig('pkg-config libzip --cflags')

def subdirs(path):
    lst = []
    for root, dirs, files in os.walk(path):
        lst += [os.path.join(root, d) for d in dirs]
    lst.sort()
    return lst

# Libraries
srcdirs = subdirs('src')
srcdirs.append('src')

sources = []
for d in srcdirs:
    sources.append(Glob(os.path.join(d, '*.c')))
    sources.append(Glob(os.path.join(d, '*.cc')))

static_lib = env.StaticLibrary('lib/ucd', sources)
shared_lib = env.SharedLibrary('lib/ucd', sources)

# Unicode Database
fetches = []
ucds = {}
for version in ['7.0.0']:
    f1 = env.Fetch('ucd/%s/UCD.zip' % version, None)
    f2 = env.Fetch('ucd/%s/Unihan.zip' % version, None)
    fetches.append(f1)
    fetches.append(f2)

    zipfile = 'ucd/%s/UCD.zip' % version
    unihanfile = 'ucd/%s/Unihan.zip' % version
    ucdfile = 'ucd/packed/unicode-%s.ucd' % version

    ucds[version] = env.Command(ucdfile, [zipfile, unihanfile],
                                'tools/ucdc %s %s $TARGET' % (version,
                                                              'ucd/%s' % version))
    env.Depends(ucds[version], [f1, f2])

env.Default([static_lib, shared_lib] + ucds.values())

# Tests
testdirs = subdirs('tests')
testdirs.append('tests')

test_sources = []
for d in testdirs:
    test_sources.append(Glob(os.path.join(d, '*.c')))
    test_sources.append(Glob(os.path.join(d, '*.cc')))

test_runner = env.Program('tests/run_tests', test_sources,
                          LIBS=['ucd'])

check = env.Command('check', None, 'tests/run_tests')
env.Depends(check, test_runner)
env.Depends(check, ucds['7.0.0'])

