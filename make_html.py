#!/usr/bin/env python3

import sys
html_template = open('charts-template.html', 'r').read()
open('charts.html', 'w').write(html_template.replace('__CHART_DATA_GOES_HERE__', sys.stdin.read()))
