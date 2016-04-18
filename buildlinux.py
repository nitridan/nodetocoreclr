import argparse
import os
import shutil
import glob
from os import path
import subprocess

script_dir = os.path.dirname(os.path.realpath(__file__))
DOTNET_SDK_URL = 'https://dotnetcli.blob.core.windows.net/dotnet/beta/Binaries/Latest/dotnet-dev-ubuntu-x64.latest.tar.gz'
ATOM_SHELL_URL = 'https://atom.io/download/atom-shell'

argument_parser = argparse.ArgumentParser(description='nodetocoreclr linux build script')
argument_parser.add_argument('-buildNumber', type=int,
                            help='Build number for binaries', default=1)
argument_parser.add_argument('-electronVersion', type=str,
                             help='Version of electron to build', default='0.37.6')
argument_parser.add_argument('-nodeVersion', type=str,
                             help='Version of node to build', default='5.10.1')

args = argument_parser.parse_args()
build_number = args.buildNumber

ELECTRON_VERSION = args.electronVersion
NODE_VERSION = args.nodeVersion
NATIVE_VERSION = '1.2.' + str(build_number)


def exit_if_failure(exit_code, error):
    if exit_code != 0:
        print error
        exit(exit_code)


def create_dist_dir(dist_dir):
    if path.exists(dist_dir):
        shutil.rmtree(dist_dir)

    os.makedirs(dist_dir)

# Build native and JS part
node_native_dir = path.join(script_dir, 'nodenative')
output_path = path.join(node_native_dir, 'build', 'Release', 'ClrLoader.node')
distributive_dir = path.join(node_native_dir, 'dist')
x86_binary_target = path.join(distributive_dir, 'ClrLoader_x86.node')
x64_binary_target = path.join(distributive_dir, 'ClrLoader_x64.node')
package_dist_json_path = path.join(node_native_dir, 'package_dist.json')
os.chdir(node_native_dir)
return_code = subprocess.call('npm install', shell=True)
exit_if_failure(return_code, 'Failed to restore npm dependencies')

return_code = subprocess.call('node-gyp rebuild --arch=x64 --target=' + NODE_VERSION, shell=True)
exit_if_failure(return_code, 'Failed to build for node.js x64')

create_dist_dir(distributive_dir)
shutil.copy(output_path, x64_binary_target)
return_code = subprocess.call('node-gyp rebuild --arch=ia32 --target=' + NODE_VERSION, shell=True)
exit_if_failure(return_code, 'Failed to build for node.js x86')

shutil.copy(output_path, x86_binary_target)
for js_file in glob.glob1(node_native_dir, '*.js'):
    shutil.copy(js_file, distributive_dir)

shutil.copy(package_dist_json_path, path.join(distributive_dir, 'package.json'))
os.chdir(distributive_dir)
return_code = subprocess.call('npm version ' + NATIVE_VERSION, shell=True)
exit_if_failure(return_code, 'Failed to set node.js package version')
return_code = subprocess.call('npm pack', shell=True)
exit_if_failure(return_code, 'Failed to create package for node.js')

# Build for electron
electron_distributive_dir = path.join(node_native_dir, 'dist-electron')
electron_x86_binary_target = path.join(electron_distributive_dir, 'ClrLoader_x86.node')
electron_x64_binary_target = path.join(electron_distributive_dir, 'ClrLoader_x64.node')
os.chdir(node_native_dir)
return_code = subprocess.call('node-gyp rebuild --arch=x64 --target=' +
                              ELECTRON_VERSION + ' --dist-url=' + ATOM_SHELL_URL,
                              shell=True)
exit_if_failure(return_code, 'Failed to build for electron x64')
create_dist_dir(electron_distributive_dir)
shutil.copy(output_path, electron_x64_binary_target)
return_code = subprocess.call('node-gyp rebuild --arch=ia32 --target=' +
                              ELECTRON_VERSION + ' --dist-url=' + ATOM_SHELL_URL,
                              shell=True)
exit_if_failure(return_code, 'Failed to build for electron x86')
shutil.copy(output_path, electron_x86_binary_target)
for js_file in glob.glob1(node_native_dir, '*.js'):
    shutil.copy(js_file, electron_distributive_dir)

shutil.copy(package_dist_json_path, path.join(electron_distributive_dir, 'package.json'))
os.chdir(electron_distributive_dir)
return_code = subprocess.call('npm version ' + NATIVE_VERSION, shell=True)
exit_if_failure(return_code, 'Failed to set electron package version')
return_code = subprocess.call('npm pack', shell=True)
exit_if_failure(return_code, 'Failed to create package for electron')