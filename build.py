import os, os.path
import shutil
import subprocess


EXT_DIR = os.path.dirname(os.path.realpath(__file__))
BUILD_DIR = os.path.join(EXT_DIR, 'build')

if not os.path.exists(BUILD_DIR):
    os.mkdir(BUILD_DIR)


print('Download sourcemod')
BRANCH = os.environ['BRANCH'] if 'BRANCH' in os.environ else '1.10-dev'
SM_PATH = os.path.join(EXT_DIR, 'sourcemod-' + BRANCH)
subprocess.call(['git', 'clone', 'https://github.com/alliedmodders/sourcemod', '--recursive', '--branch', BRANCH, '--single-branch', SM_PATH])


print('Install ambuild')
os.chdir(os.path.join(EXT_DIR, 'ambuild'))
subprocess.call(['python', 'setup.py', 'install'])


print('Setup environment')
if os.name == 'nt':
    subprocess.call(['cmd', '/C', os.path.join(EXT_DIR, 'vcpkg', 'bootstrap-vcpkg.bat')])
else:
    subprocess.call(['sh', os.path.join(EXT_DIR, 'vcpkg', 'bootstrap-vcpkg.sh')])

VCPKG_ROOT = os.path.join(EXT_DIR, 'vcpkg')
VCPKG_TARGET_TRIPLET = 'x86-linux-sm'
if os.name == 'nt':
    VCPKG_TARGET_TRIPLET = 'x86-windows-sm'

os.environ['VCPKG_ROOT'] = VCPKG_ROOT
shutil.copyfile(os.path.join(EXT_DIR, 'vcpkg_triplet', VCPKG_TARGET_TRIPLET+'.cmake'), os.path.join(VCPKG_ROOT, 'triplets', VCPKG_TARGET_TRIPLET+'.cmake'))


print('Setup vcpkg packages')

VCPKG_EXEC = os.path.join(VCPKG_ROOT, 'vcpkg')
if os.name == 'nt':
    VCPKG_EXEC += '.exe'

subprocess.call([VCPKG_EXEC, 'install', 'boost-asio', 'boost-beast', 'openssl', 'jsoncpp', '--triplet', VCPKG_TARGET_TRIPLET])

print('Start building')
os.chdir(BUILD_DIR)
subprocess.call(['python', '../configure.py', '--enable-optimize', '--sm-path', SM_PATH])
subprocess.call(['ambuild'])
