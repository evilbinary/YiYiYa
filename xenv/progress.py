# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os
import subprocess
import math
import json

from SCons.Script import *


class ProgressCounter(object):
    count = 0
    env = None

    def __init__(self, env):
        self.env = env

    def __call__(self, node, *args, **kw):
        self.count += 1
        env=self.env
        num_targets = len(self.env.fs.Dir('.').children())

        sys.stderr.write('Evaluated %s %s nodes %d\n' % (self.count,node, num_targets))


def my_progress_function(node, *args, **kwargs):
    print('CC %s' % node)


class my_progress:
    count = 0

    def __call__(self, node, *args, **kw):
        self.count = self.count + 1
        sys.stderr.write('%s: %s\n' % (self.count, node))


def progress_settings(env, set):
    # Progress(my_progress())
    Progress(['-\r', '\\\r', '|\r', '/\r'], interval=5)
    # Progress(my_progress_function, interval=10)
    # Progress(ProgressCounter(env), interval=100)
