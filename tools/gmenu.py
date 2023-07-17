# coding:utf-8
import os
import sys
import re
import json


path='app/resource/gmenu2x/skins/480x272/Default/icons/'
target_path='app/resource/gmenu2x/sections/applications/'

file_or_dir = os.listdir(path)
for file_dir in file_or_dir:
    if not os.path.exists(target_path+ file_dir):
        file_name= file_dir.split(".")[0]
        print('gen ',file_name)
        with open(target_path+ file_name, "w") as file:
            section="""title=%s
icon=skin:icons/%s
exec=/usr/bin/%s
wrapper=true"""%(file_name,file_dir,file_name)
            file.write(section)

