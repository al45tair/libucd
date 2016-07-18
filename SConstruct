# Emacs, this is -*-python-*-
import os, os.path
import urllib

def fetchucd(target, source, env):
    abspath = target[0].get_abspath()
    vers, fname = abspath.rsplit('/', 2)[-2:]
    url = 'http://www.unicode.org/Public/zipped/%s/%s' % (vers, fname)
    print 'Fetching %s' % url
    urllib.urlretrieve(url, abspath)

fetchucd_bld = Builder(action=fetchucd)

def fetchemj(target, source, env):
    abspath = target[0].get_abspath()
    vers, fname = abspath.rsplit('/', 2)[-2:]
    url = 'http://www.unicode.org/Public/emoji/%s/%s' % (vers, fname)
    print 'Fetching %s' % url
    urllib.urlretrieve(url, abspath)

fetchemj_bld = Builder(action=fetchemj)

env = Environment(BUILDERS = {'FetchUcd': fetchucd_bld,
                              'FetchEmoji': fetchemj_bld})

# Compiler options

env['CC'] = os.getenv('CC') or 'clang'
env['CXX'] = os.getenv('CXX') or 'clang++'

if env['CXX'] == 'clang++':
    stdflags = '-std=c++14 -stdlib=libc++'
else:
    stdflags = '--std=c++14'

env['CFLAGS'] = os.getenv('CFLAGS') or '-g -W -Wall -Wno-multichar'
env['CXXFLAGS'] = os.getenv('CXXFLAGS') or ' '.join([env['CFLAGS'], stdflags])

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
for version in [('9.0.0', '3.0')]:
    ucdver = version[0]
    emjver = version[1]
    f1 = env.FetchUcd('ucd/%s/UCD.zip' % ucdver, None)
    f2 = env.FetchUcd('ucd/%s/Unihan.zip' % ucdver, None)
    f3 = env.FetchEmoji('emoji/%s/emoji-data.txt' % emjver, None)
    fetches.append(f1)
    fetches.append(f2)
    fetches.append(f3)

    zipfile = 'ucd/%s/UCD.zip' % ucdver
    unihanfile = 'ucd/%s/Unihan.zip' % ucdver
    emjfile = 'emoji/%s/emoji-data.txt' % emjver
    ucdfile = 'ucd/packed/unicode-%s.ucd' % ucdver
    ucds[ucdver] = env.Command(ucdfile, [zipfile, unihanfile, emjfile],
                               'tools/ucdc %s %s %s %s $TARGET' \
                               % (ucdver, 'ucd/%s' % ucdver,
                                  emjver, 'emoji/%s' % emjver ))
    env.Depends(ucds[ucdver], [f1, f2, f3])

env.Default([static_lib] + ucds.values())

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
env.Depends(check, ucds['9.0.0'])

