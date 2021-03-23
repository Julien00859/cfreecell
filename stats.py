#!/usr/bin/env python3

import asyncio
import itertools
import functools
import os
import pathlib
import subprocess as sp
import signal
import sys

WORKERS = os.cpu_count() - 1  # Let's be nice and leave one cpu free
ROOT = pathlib.Path(__file__).resolve().absolute().parent
counter = itertools.count(int(sys.argv[1]) if len(sys.argv) > 1 else 0)
times_path = ROOT / 'stats' / 'times.txt'


async def run(event):

    while not event.is_set():
        seed = next(counter)
        with (ROOT / 'stats' / f'game_{seed:06d}.txt').open('wb') as fd:
            proc = await asyncio.create_subprocess_exec(
                '/usr/bin/timeout', '-v', '10',
                '/usr/bin/time', '-a', '-o', str(times_path),
                '-f', f'seed {seed:06d} code %x wall %e user %U sys %S mem %Mkb',
                str(ROOT / 'freecell'), str(seed),
                stdout=fd, stderr=fd)
            await proc.wait()
            if proc.returncode != 0:
                with times_path.open('a') as fd:
                    fd.write(f'seed {seed:06d} code -15 wall ?? user ?? sys ?? mem ??kb\n')


def main():
    times_path.parent.mkdir(exist_ok=True)
    loop = asyncio.new_event_loop()
    event = asyncio.Event()
    @functools.partial(signal.signal, signal.SIGINT)
    def stop(*_):
        event.set()
        print(" Stopping... ", file=sys.stderr)
    loop.run_until_complete(asyncio.wait([run(event) for _ in range(WORKERS)]))

main()
