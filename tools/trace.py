# coding:utf-8
import os
import sys
import re
import json
from collections import OrderedDict

# pip3 install pyelftools
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection



def static_func(file_path):
    pattern = r"\[(\w+)\] (\w+) t:\w+ f:(\w+) c:(\w+) l:(\w+)"
    log_data = ''
    function_counts = {}
    function_e_cycles = {}
    function_x_cycles = {}
    with open(file_path, "r") as file:
        log_data = file.read()
    matches = re.findall(pattern, log_data)

    for match in matches:
        cycles,t,func_addr, caller_addr,level = match
        function_cycles=function_e_cycles
        if t=='x':
            function_cycles=function_x_cycles
        
        print('cycles=>',cycles,function_cycles)

        if func_addr in function_counts:
            function_counts[func_addr] += 1
            function_cycles[func_addr] += int(cycles)
        else:
            function_counts[func_addr] = 1
            function_cycles[func_addr] = int(cycles)
    return (function_counts, function_x_cycles)


def find_symbol_by_address(symbol_table, address):
    # 遍历符号表中的所有符号项
    symbols = symbol_table.iter_symbols()
    for symbol in symbols:
        # print( type(symbol['st_value']),'===',address , symbol.name)
        if symbol['st_value'] == int('0x'+address, 16):
            return symbol.name  # 找到匹配的符号名称

    return None  # 未找到匹配的符号名称


symbols_cache={}

def get_symbol(f, address):
    ret = None
    if symbols_cache.get(address):
        return symbols_cache.get(address)
    elf = ELFFile(f)
    # 遍历 ELF 文件中的所有符号表
    for section in elf.iter_sections():
        if isinstance(section, SymbolTableSection):  # 找到符号表节
            symbol_table = section
            ret = find_symbol_by_address(symbol_table, address)
            if ret:
                symbols_cache[address]=ret
                return ret
    return ret


file_path = "trace.txt"  # 文件路径

# function_counts, function_cycles = static_func(file_path)

# formatted_data = json.dumps(function_counts, indent=4)
# # print(formatted_data)

# sorted_counts = OrderedDict(
#     sorted(function_counts.items(), key=lambda x: x[1], reverse=True))
# sorted_cycels = OrderedDict(
#     sorted(function_cycles.items(), key=lambda x: x[1], reverse=True))


# formatted_data = json.dumps(sorted_counts, indent=4)
# print(formatted_data)

# formatted_data = json.dumps(sorted_cycels, indent=4)
# print(formatted_data)


elf_filepath = '../duck/init/kernel.elf'
f = open(elf_filepath, 'rb')

# print('counts')
# for func_addr, count in sorted_counts.items():
#     symbol_name = get_symbol(f, func_addr)
#     print(func_addr,'count:', count, 'cycles:',
#           function_cycles[func_addr], ' avg:',function_cycles[func_addr]/count, symbol_name)

# print('')
# print('cycles')

# for func_addr, cycles in sorted_cycels.items():
#     symbol_name = get_symbol(f, func_addr)
#     print(func_addr, 'cycles:',cycles, 'counts:',function_counts[func_addr], ' avg:',cycles/function_counts[func_addr],symbol_name)

# print('')
# print('avg')


# f.close()


# ANSI escape sequences for different colors
RESET = "\033[0m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
BLUE = "\033[34m"
MAGENTA = "\033[35m"
CYAN = "\033[36m"
WHITE = "\033[37m"

# e表示进去，x表示出去 ，l表示层级，f表示当前函数地址，c：表示调用者地址

log = """
[2307730716] x t:2 f:4201062c c:42010fec l:12
[2313911798] e t:2 f:42010acc c:42011020 l:12
[2321670588] x t:2 f:42010acc c:42011020 l:12
[2327891827] x t:2 f:42010f40 c:42011570 l:11
[2334117955] x t:2 f:42011500 c:42011e1c l:10
[2340316073] e t:2 f:42011bcc c:42011e58 l:10
[2346500874] e t:2 f:42011500 c:42011c64 l:11
[2352697297] e t:2 f:4201062c c:42011558 l:12
[2358907995] x t:2 f:4201062c c:42011558 l:12
[2365093243] x t:2 f:42011500 c:42011c64 l:11
[2371294139] x t:2 f:42011bcc c:42011e58 l:10
[2377475110] e t:2 f:42011500 c:42011ed4 l:10
[2383679395] e t:2 f:4201062c c:42011558 l:11
"""


log_data=''

with open(file_path, "r") as file:
    log_data = file.read()
enter_times = {}  # 记录函数进入时间

for line in log_data.split("\n"):
    line = line.strip()
    if line:
        time = int(line.split("[")[1].split("]")[0])
        func_addr = line.split("f:")[1].split(" ")[0]
        event = line.split(" ")[1]

        if event == "e":
            enter_times[func_addr] = time
        elif event == "x":
            if func_addr in enter_times:
                enter_time = enter_times.pop(func_addr)
                level = line.split("l:")[1].split(" ")[0]
                indent = "  " * int(level)  # 根据调用级别添加缩进
                duration = time - enter_time
                func = get_symbol(f, func_addr)
                print(f"{indent}- Function: {func_addr} {RED}{func}{RESET}, Enter Time: {enter_time}, Exit Time: {time}, Duration: {duration} {GREEN}{duration/1000/1000}{RESET}")

# 处理未离开的函数
for func_addr, enter_time in enter_times.items():
    func = get_symbol(f, func_addr)
    print(f"- Function: {func_addr} {func}, Enter Time: {enter_time}, Not Exited")

