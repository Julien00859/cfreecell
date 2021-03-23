#!/usr/bin/env python3

import asyncio
import itertools
import os
import pathlib
import subprocess as sp
import signal
import sys

WORKERS = os.cpu_count() - 3
ROOT = pathlib.Path(__file__).resolve().absolute().parent
counter = itertools.count(int(sys.argv[1]) if len(sys.argv) > 1 else 0)


async def run(event):
    while not event.is_set():
        seed = next(counter)
        with (ROOT / 'stats' / f'game_{seed:06d}.txt').open('wb') as fd:
            proc = await asyncio.create_subprocess_exec(
                '/usr/bin/time',
                  '-a', '-o', str(ROOT / 'stats' / 'times.txt'),
                  '-f', f'seed {seed:06d} code %x wall %e user %U sys %S mem %Mkb',
                  str(ROOT / 'freecell'), str(seed),
                stdout=fd, stderr=fd)
            try:
                await asyncio.wait_for(proc.wait(), 10)
            except asyncio.TimeoutError:
                proc.terminate()
                try:
                    await asyncio.wait_for(proc.wait(), 1)
                except asyncio.TimeoutError:
                    proc.kill()


def main():
    (ROOT / 'stats').mkdir(exist_ok=True)

    loop = asyncio.new_event_loop()
    event = asyncio.Event()
    signal.signal(signal.SIGINT, lambda *_: event.set())
    loop.run_until_complete(asyncio.wait([run(event) for _ in range(WORKERS)]))

main()
