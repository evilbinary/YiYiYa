# coding:utf-8
# *******************************************************************
# * Copyright 2021-2080 evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
import os
import math
import platform 
import sys

Import('env')

SConscript(dirs=['libs/libalgorithm'], exports='env')
SConscript(dirs=['platform'], exports='env')
SConscript(dirs=['arch'], exports='env')
SConscript(dirs=['kernel'], exports='env')
SConscript(dirs=['libs/libkernel'], exports='env')
SConscript(dirs=['libs/libarchcommon'], exports='env')

SConscript(dirs=['modules'], exports='env')
kernel=SConscript(dirs=['init'], exports='env')



