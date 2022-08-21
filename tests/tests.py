# this is the test runner
# all of the other files contain simple tests
# than prevent regressions

import os
import pathlib
import time

class Colors:
    reset = '\033[0m'
    bold = '\033[01m'
    disable = '\033[02m'
    underline = '\033[04m'
    reverse = '\033[07m'
    strikethrough = '\033[09m'
    invisible = '\033[08m'

    class FG:
        black = '\033[30m'
        red = '\033[31m'
        green = '\033[32m'
        orange = '\033[33m'
        blue = '\033[34m'
        purple = '\033[35m'
        cyan = '\033[36m'
        lightgrey = '\033[37m'
        darkgrey = '\033[90m'
        lightred = '\033[91m'
        lightgreen = '\033[92m'
        yellow = '\033[93m'
        lightblue = '\033[94m'
        pink = '\033[95m'
        lightcyan = '\033[96m'

test_dir = os.getcwd()

COMPILER_BUILD_CMD_BASE = 'yupc build'
compiler_cmd = COMPILER_BUILD_CMD_BASE

compiler_options = []

class CompilerOption:
    name = ''
    value = ''

    def __init__(self, name, value):
        self.name = name
        self.value = value

def append_option(x):
    compiler_options.append(x)

def merge_options(opts: list[CompilerOption]):
    global compiler_cmd

    for o in opts:
        compiler_cmd = compiler_cmd + ' '
        compiler_cmd = compiler_cmd + o.name
        compiler_cmd = compiler_cmd + ' '
        compiler_cmd = compiler_cmd + o.value

for test in os.listdir(test_dir):
    t0 = time.time()

    f = os.path.join(test_dir, test)

    ext = pathlib.Path(f).suffix
    if os.path.isfile(f) and ext == '.yup':
        base_name = os.path.basename(f)
        fname_wo_ext = os.path.splitext(base_name)[0]
        print(f'RUNNING TEST: {Colors().FG().orange}{Colors().underline} {fname_wo_ext} {Colors().reset}')

        s_option = CompilerOption('-s', f)
        v_option = CompilerOption('-v', '')
        b_option = CompilerOption('-b', f'test.{base_name}.bc')

        append_option(s_option)
        append_option(v_option)
        append_option(b_option)

        merge_options(compiler_options)

        print(f'BUILD CMD: {Colors().FG().darkgrey}{compiler_cmd}{Colors().reset}')

        EC = os.system(compiler_cmd)

        t1 = time.time()
        finish_time = t1 - t0

        print(f'FINISHED TEST: EC = {EC}, time = {finish_time}')
