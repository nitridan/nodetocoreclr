import os
import urllib2
from os import path
import argparse
import traceback

SUCCESS_EXIT_CODE = 0
FAILED_TO_UPLOAD_EXIT_CODE = 1
FAILED_TO_LOAD_FILE_EXIT_CODE = 2
UNKNOWN_ERROR_EXIT_CODE = 3
DROPBOX_UPLOAD_ROOT = 'https://content.dropboxapi.com/1/files_put/auto/'

def publish_file(token, path, upload_name):
    try:
        with open(path, 'rb') as artifact:
            data=artifact.read()

        url = DROPBOX_UPLOAD_ROOT + upload_name
        headers= {'Authorization':'Bearer ' + token, 'Content-Type': 'application/octet-stream'}
        request = urllib2.Request(url, data, headers)
        response = urllib2.urlopen(request)
        print 'Upload succeed'
        print response.read()
    except urllib2.HTTPError as e:
        print 'Failed to upload file'
        print 'HTTP code: ' + e.code
        print 'Reason: ' + e.reason
        exit(FAILED_TO_UPLOAD_EXIT_CODE)
    except IOError:
        print 'Failed to read file'
        exit(FAILED_TO_LOAD_FILE_EXIT_CODE)
    except:
        print 'Unknown error'
        traceback.print_exc()
        exit(UNKNOWN_ERROR_EXIT_CODE)


argument_parser = argparse.ArgumentParser(description='Script for uploading nodetocoreclr artifacts to dropbox')
argument_parser.add_argument('-version', required=True, type=str, help='Artifacts version')
argument_parser.add_argument('-token', type=str, required=True, help='Dropbox token')
argument_parser.add_argument('-uploadNuget', help='Version of electron to build', action='store_true')

args = argument_parser.parse_args()
version = args.version
upload_nuget = args.uploadNuget
dropbox_token = args.token
script_dir = path.dirname(path.realpath(__file__))

os_suffix = 'win' if os.name == 'nt' else 'linux'
file_name = 'nodetocoreclr-' + version + '.tgz'
node_native_dir = path.join(script_dir, 'nodenative')
node_file_path = path.join(node_native_dir, 'dist', file_name)
node_upload_name = 'nodetocoreclr-node-' + os_suffix + '-' + version + '.tgz'
print 'Publishing node.js artifact'
publish_file(dropbox_token, node_file_path, node_upload_name)

electron_file_path = path.join(node_native_dir, 'dist-electron', file_name)
electron_upload_name = 'nodetocoreclr-electron-' + os_suffix + '-' + version + '.tgz'
print 'Publishing electron artifact'
publish_file(dropbox_token, electron_file_path, electron_upload_name)

if upload_nuget != True:
    exit(SUCCESS_EXIT_CODE)

nuget_name = 'Nitridan.CoreClrNode.' + '.'.join(version.split('.')[0:2]) + '-release-' + version.split('.')[-1:][0] + '.nupkg'
nuget_path = path.join(script_dir, 'coreclrnode', 'bin', 'Release', nuget_name)
print 'Uploading nuget package'
publish_file(dropbox_token, nuget_path, nuget_name)