#!/usr/bin/env python3


import argparse
import pathlib
import random
import requests
import sys
import time
from bs4 import BeautifulSoup as bs

gameurl = "https://freecellgamesolutions.com/fcs/?game=%d"
solurl = "https://freecellgamesolutions.com/std/?g=%d&v=All"
useragent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:85.0) Gecko/20100101 Firefox/85.0"
root = pathlib.Path(__file__).resolve().parent


def fix_markup(html):
    return html.replace('<td', '</td><td').replace('</td><td', '<td', 1)


def get_game(gameid):
    res = requests.get(gameurl % gameid, headers={'User-Agent': useragent})
    soup = bs(fix_markup(res.text), 'html.parser')
    table = soup.find(id='TableLooksLikeThis')

    return "\n".join(
        " ".join(
            format(td.text, '>3')
            for td in
            tr.find_all('td')
        )
        for tr
        in table.find_all('tr')
    )


def get_solution(gameid):
    res = requests.get(solurl % gameid, headers={'User-Agent': useragent})
    soup = bs(fix_markup(res.text), 'html.parser')
    trs = [
        tag
        for tag
        in soup.find(href='/notation.html').parent.parent.previous_siblings
        if tag.name == 'tr'
    ]
    trs.reverse()

    return "\n".join(
        " ".join(td.text for td in tr.find_all('td'))
        for tr in trs
    )


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('action', choices=['game', 'soluce'])
    parser.add_argument('gameid', type=int)
    parser.add_argument('--force', action='store_true')
    opt = parser.parse_args()
    path = root.joinpath('data', f'{opt.action}{opt.gameid}')
    if not opt.force and path.exists():
        sys.exit("file exists already")

    data = {'game': get_game, 'soluce': get_solution}[opt.action](opt.gameid)
    print(data)
    with root.joinpath('data', f'{opt.gameid}-{opt.action}.txt').open('w') as fd:
        fd.write(data.translate({
            ord('♣'): 'C',
            ord('♥'): 'H',
            ord('♦'): 'D',
            ord('♠'): 'S'
        }))
        fd.write('\n')

if __name__ == '__main__':
    main()
