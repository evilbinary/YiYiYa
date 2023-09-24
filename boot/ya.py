# coding:utf-8
# *******************************************************************
# * Copyright 2021-present evilbinary
# * 作者: evilbinary on 01/01/20
# * 邮箱: rootdebug@163.com
# ********************************************************************
arch_type=get_arch_type()

if arch_type!='general': 
    if arch_type and arch_type!='':
        includes("./"+arch_type+"/ya.py")
