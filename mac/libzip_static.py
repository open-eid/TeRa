""" Uncomment static target from libzip CMakeLists.txt """
from __future__ import print_function
import fileinput
import os
import sys


def remove_static_comments(path):
    """ Uncomment block of lines starting with '#ADD_LIBRARY(zipstatic' """
    search_for_block = True
    uncomment = False
    for line in fileinput.input(path, inplace=True):
        if search_for_block and line.startswith('#ADD_LIBRARY(zipstatic'):
            uncomment = True
            search_for_block = False
        if uncomment and not line.startswith('#'):
            uncomment = False
        print(line if not uncomment else line.lstrip('#'), end='')


if __name__ == '__main__':
    UNCOMMENT = sys.argv[1] if len(sys.argv) > 1 else ''
    if os.path.isfile(UNCOMMENT) and os.access(UNCOMMENT, os.W_OK):
        remove_static_comments(UNCOMMENT)
    else:
        print('libzip cmakefile ''%s'' is not writable' % UNCOMMENT)