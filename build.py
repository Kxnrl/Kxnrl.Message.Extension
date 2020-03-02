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

print('Start building')
os.chdir(BUILD_DIR)
subprocess.call(['python', '../configure.py', '--enable-optimize', '--sm-path', SM_PATH])
subprocess.call(['python', '-m', 'ambuild'])
