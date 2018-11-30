#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import shutil

src_parent = 'F:\\chromium\\src'
dest_parent = 'F:\\chromium\\src'

common_suffixs = ['typemaps.gni', '.typemap', '.mojom', 'features.gni']

special_case = [
    'third_party/Webkit',
    '//ui/views/examples/BUILD.gn  content',
    '//ui/events/BUILD.gn blink',
    '//ui/resources/BUILD.gn: blink',
    ' //mojo/public/cpp/bindings/BUILD.gn: blink',
    '//ui/events/blink/BUILD.gn: blink',
    '//ui/events/gestures/blink/BUILD.gn blink',
    ' //tools/v8_context_snapshot/BUILD.gn'
    
    
    '//components/test/BUILD.gn: content',
    '//components/services/pdf_compositor/BUILD.gn: content',
    '//components/metrics/BUILD.gn:',

    '//components/ukm/BUILD.gn: components'
]

'''
allfolder = [
    ['third_party\\blink', ['renderer\\config.gni', 'platform\\mojo\\BUILD.gn', 'renderer\\core\\core.gni']],
    ['android_webview', []],
    

    
    ['chrome', ['VERSION', 'BRANDING']],
    ['chrome_elf', []],
    ['chromecast', []],
    ['chromeos', []],
    ['content', []],
    
]
'''

allfolder = [
    ['components\\sync', []]
]

new_prefix = 'zzz__' 

def needcopy(filename, suffixs):
    for suffix in suffixs:
        if filename.endswith(suffix):
            return True
    
    return False


def handlecommon(src, dest, suffixs):
    if not os.path.exists(src):
        return

    for (root, dirs, files) in os.walk(src):
        for filename in files:
            src_path = os.path.join(root, filename)
            if needcopy(src_path, suffixs):
                dest_path = src_path
                dest_path = dest_path.replace(src, dest)
                if os.path.exists(dest_path):
                    continue
                dest_parent_path = os.path.dirname(dest_path)
                if not os.path.exists(dest_parent_path):
                    os.makedirs(dest_parent_path)
                shutil.copyfile(src_path, dest_path)
                print dest_path
            else:
                continue


def handle_one_folder(dest_name, more_suffixs):
    if not dest_name:
        return

    dest = os.path.join(dest_parent, dest_name)
    dest_file_name = os.path.basename(dest)

    src_name = dest_name.replace(dest_file_name, new_prefix + dest_file_name)
    src = os.path.join(src_parent, src_name)
    
    suffixs = common_suffixs + more_suffixs
    handlecommon(src, dest, suffixs)

def main():
    # handlechrome()
    # handle_webkit()
    handle_components()
    for item in allfolder:
        handle_one_folder(item[0], item[1])


def handle_components():
    for file_name in os.listdir(r'F:\chromium\src\components'):
        if file_name.startswith(new_prefix):
            old_file_name = file_name.replace(new_prefix, '')
            old_file_name = 'components\\' + old_file_name
            handle_one_folder(old_file_name, [])


if __name__ == '__main__':
    try:
        main()
    except:
        pass
