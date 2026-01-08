#!/usr/bin/env python3

import argparse
import concurrent.futures
import queue
import subprocess
import sys
import threading

from pathlib import Path

class OdamexProcess:
    def __init__(self, process):
        self._process       = process
        self._in_queue      = queue.SimpleQueue()
        self._out_queue     = queue.SimpleQueue()
        self._is_shutdown   = threading.Event()

        self._poll_result = None

        self._in_thread  = threading.Thread(target=self.forward_input)
        self._out_thread = threading.Thread(target=self.forward_output)

        self._in_thread.start()
        self._out_thread.start()

    def poll(self):
        poll_result = self._process.poll()
        if poll_result is not None and self._poll_result is None:
            self._poll_result = poll_result

            self._is_shutdown.set()
            self._in_queue.put('\n')    # Force a wakeup.
            self._in_thread.join()
            self._out_thread.join()

        return poll_result

    def forward_input(self):
        while True:
            input_line = self._in_queue.get()
            if self._is_shutdown.is_set():
                break
            self._process.stdin.write(input_line)
            self._process.stdin.flush()

    def forward_output(self):
        for line in self._process.stdout:
            self._out_queue.put(line)

    def send_input(self, input_line):
        self._in_queue.put(input_line)

    def get_output(self):
        try:
            return self._out_queue.get_nowait()
        except:
            return None


class OdamexClient:
    def __init__(self,
                 executable: str | Path,
                 cwd:        str | Path | None = None):
        if not isinstance(executable, Path):
            executable = Path(executable)

        if cwd is not None and not isinstance(cwd, Path):
            cwd = Path(cwd)

            if not cwd.is_dir():
                raise FileNotFoundError(f"No such directory to be used for CWD: {cwd}")

        if not executable.is_file():
            raise FileNotFoundError(f"No such Odamex executable: {executable}")


        self.executable = executable.resolve()
        self.cwd        = cwd.resolve() if cwd is not None else self.executable.parent

    def launch(self, *args):

        process = subprocess.Popen([ self.executable ] + list(args),
                                   cwd    = self.cwd,
                                   text   = True,
                                   stdin  = subprocess.PIPE,
                                   stdout = subprocess.PIPE,
                                   stderr = subprocess.STDOUT,
                                   )
        return OdamexProcess(process)

class Snoopy:
    def __init__(self, client_process):
        self.client_process = client_process

    def update(self):

        result = self.client_process.poll()

        output_line = self.client_process.get_output()

        if output_line is not None:
            sys.stdout.write(output_line)
            if output_line.endswith(" now\n"):
                commander_name = output_line.split(':')[0]

                self.client_process.send_input(f"spy {commander_name}\n")
            elif output_line.endswith(" quit\n"):
                self.client_process.send_input(f"quit\n")

        return result

def main():
    parser = argparse.ArgumentParser(description="Launch a client with its stdin/out/err hooked up to the local terminal.")
    parser.add_argument('executable',        type=str,            help="The odamex client executable to run.")
    parser.add_argument('--no-print-stdout', action='store_true', help="Suppress output from the client.  Please note that this will prevent the script from being able to understand chat commands.")
    parser.add_argument('--cwd',             type=str,            help="The current working directory to use.  Defaults to the executable's directory.")

    args, remaining_args = parser.parse_known_args()

    client = OdamexClient(args.executable, args.cwd)

    if not args.no_print_stdout:
        remaining_args += ['+print_stdout','1']

    snooper = Snoopy(client.launch(*remaining_args))

    while True:
        if snooper.update() is not None:
            break

if __name__ == '__main__':
    main()
