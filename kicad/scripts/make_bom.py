#
# Python script to generate the BOM from the netlist
#
'''
    @package
    Generate a HTML BOM.
    Components are sorted by ref and grouped by value
    Fields are
      Ref, Quantity, Value, Rating, Description, Footprint, Criticality, Vendor, Part No, Octopart, Reichelt, Price, Subtotal
'''


from __future__ import print_function
from __future__ import unicode_literals

import codecs
import collections
import itertools
import json
import re
import sys

import six

from lxml import etree
import requests

def tr (rows):
    count = 0
    trs = []
    for row in rows:
        class_ = 'odd' if count & 0x01 else 'even'
        count += 1
        trs.append (('<tr class="%s">' % class_) + row + '</tr>')
    return '\n'.join (trs)


def th (elems):
    elems = [('<th class="%s">' % k) + k + '</th>' for k, v in elems.items ()]
    return '\n'.join (elems)


def td (elems):
    res = []
    for k, v in elems.items ():
        class_ = k
        if k == 'cr':
            class_ += ' ' + v
            v = ''
        v = v or ''
        res.append (('<td class="%s">' % class_) + v + '</td>')
    return '\n'.join (res)


def strip_lib (val):
    pos = val.find (':')
    if pos > -1:
        return val[pos + 1:]
    return val


def make_row ():
    return collections.OrderedDict.fromkeys (
        'refs qty value rating description footprint cr vendor partno octopart reichelt price subtotal'.split ())


def to_range (values):
    if not values:
        return []

    ranges = []

    m = re.match ('(\w+)(\d+)', values[0])
    if m:
        prefix     = m.group (1)
        prefix_len = len (prefix)

        last_val = 0
        r = [0,0]
        for value in values:
            val = int (value[prefix_len:])

            if r[0] == 0:
                r[0] = val
            elif r[1] < val - 1:
                ranges.append (r)
                r = [val,0]
            r[1] = val
        ranges.append (r)

    res = ''
    for r in ranges:
        res += prefix + str (r[0])
        if r[0] == r[1] - 1:
            res += ',' + prefix + str (r[1])
        elif r[0] != r[1]:
            res += '-' + prefix + str (r[1])
        res += ','

    return res.strip (',')


def get_reichelt (reichelt_id, cache):
    reichelt = { 'id' : reichelt_id }
    cache.setdefault ('reichelt', {})
    c_r = cache['reichelt']
    if reichelt_id in c_r:
        return c_r[reichelt_id]

    r = requests.get ('https://www.reichelt.de/?ARTICLE=%s' % reichelt_id)
    tree = etree.parse (six.StringIO (r.text), reichelt_parser)
    for div in tree.xpath ('//div[@id="article"]'):
        for meta in div.xpath ('//meta[@itemprop="price"]'):
            reichelt['price'] = float (meta.get ('content'))
        for div in div.xpath ('//div[@id="av_price_artnr"]'):
            reichelt['article'] = div.text.replace ('Artikel-Nr.: ', '')
    c_r[reichelt_id] = reichelt
    return reichelt


html = '''
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
    <head>
        <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
        <title>BOM for <!--TITLE--></title>
        <style type="text/css">
          .qty,
          .price,
          .subtotal,
          .sheet-total,
          .total { text-align: right }
          .cr { width: 1em }
          td.cr { background: #8f8 }
          td.critical { background: red }
          td.important { background: yellow }
          th { text-align: left }
          table { border-spacing: 0px }
          table.legend { margin-bottom: 1em }
          th, td { padding: 0.2em 0.5em }
          tr.odd { background-color: #efe }
        </style>
    </head>
    <body>
    <h1>Bill of Materials for <!--TITLE--></h1>
    <table class="legend">
      <tr><td class="cr critical"></td><td>Critical: use only specified vendor and part no.</td></tr>
      <tr><td class="cr important"></td><td>Important: substitute with part of equivalent specifications and dimensions</td></tr>
      <tr><td class="cr"></td><td>Non-Critical: select equivalent component by cost and availability</td></tr>
    </table>
    <table>
      <!--ROWS-->
    </table>
    </body>
</html>
    '''

fn_xml   = sys.argv[1]
fn_bom   = sys.argv[2] + '.bom.html'
fn_cache = sys.argv[2] + '.bom-cache.json'

netlist_parser = etree.XMLParser ()
reichelt_parser = etree.HTMLParser ()

with open (fn_xml) as fp:
    netlist = etree.parse (fp, netlist_parser)

def key (c):
    return (
        get_sheet (c) +
        get_class (c) +
        get_field (c, 'value') +
        c.get ('rating') +
        get_user_field (c, 'Criticality') +
        get_field (c, 'footprint')
    )

def sort_by_ref (c1, c2):
    return cmp (c1.get ('sortref'), c2.get ('sortref'))

def get_field (c, name):
    f = c.find (name)
    if f is not None:
        return f.text

    libsource = c.find ('libsource')
    if libsource is not None:
        lib = libsource.get ('lib')
        part = libsource.get ('part')
        for libpart in netlist.xpath ('//libpart[@lib="%s" and @part="%s"]' % (lib, part)):
            return get_field (libpart, name)

    return ''

def get_user_field (c, name):
    # <libsource lib="device" part="R"/>
    # <libpart lib="conn" part="CONN_01X03">

    f = c.find ('.//field[@name="%s"]' % name)
    if f is not None:
        return f.text

    libsource = c.find ('libsource')
    if libsource is not None:
        lib = libsource.get ('lib')
        part = libsource.get ('part')
        for libpart in netlist.xpath ('//libpart[@lib="%s" and @part="%s"]' % (lib, part)):
            return get_user_field (libpart, name)

    return ''

def get_class (c):
    ref = c.get ('ref')
    return re.sub ('\d+', '', ref)

def get_sheet (c):
    s = c.find ('sheetpath')
    if s is not None:
        return s.get ('names')
    return ''

def make_ref_sortable (ref):
    return re.sub ('(\d+)', lambda m: '{0:d}{1:s}'.format (len (m.group (0)), m.group (0)), ref)


components = netlist.xpath ('//comp[@ref]')

for c in components:
    c.set ('rating',  get_user_field (c, 'Rating'))
    c.set ('sortref', make_ref_sortable (c.get ('ref')))
    c.set ('key',     key (c))

rows = []
total = 0

cache = {}
try:
    with open (fn_cache, 'r') as fp:
        cache = json.load (fp)
except:
    pass

components = sorted (components, key = lambda c: c.get ('key'))

for sheet, sheet_group in itertools.groupby (components, lambda c: get_sheet (c)):
    sheet_group = list (sheet_group)

    title = netlist.find ('//sheet[@name="%s"]//title' % sheet).text
    rows.append ('<th colspan="10">%s</th>' % title)
    rows.append (th (make_row ()))
    sheet_total = 0
    sheet_rows  = []

    for key, group in itertools.groupby (sheet_group, lambda c: c.get ('key')):
        group = sorted (group, key = lambda c: c.get ('sortref'))

        c           = group[0]
        qty         = len (group)
        description = None
        octopart_id = None
        reichelt_id = None
        vendor      = None
        partno      = None
        cr          = None
        refs        = []
        row         = make_row ()

        for c in group:
            refs.append (c.get ('ref'))
            description = description or get_user_field (c, 'Description') or get_field (c, 'description')
            vendor      = vendor      or get_user_field (c, 'Vendor')
            partno      = partno      or get_user_field (c, 'PartNo')
            cr          = cr          or get_user_field (c, 'Criticality')
            octopart_id = octopart_id or get_user_field (c, 'Octopart')
            reichelt_id = reichelt_id or get_user_field (c, 'Reichelt')

        row['refs']        = to_range (refs)
        row['qty']         = str (qty)
        row['value']       = get_field (c, 'value')
        row['rating']      = c.get ('rating')
        row['footprint']   = strip_lib (get_field (c, 'footprint').replace ('_', ' '))
        row['description'] = description
        row['vendor']      = vendor
        row['partno']      = partno
        row['cr']          = cr

        if reichelt_id:
            reichelt = get_reichelt (reichelt_id, cache)
            if reichelt:
                price = reichelt.get ('price', 0)
                subtotal = qty * price
                row['reichelt']  = '<a href="https://www.reichelt.de/?ARTICLE={0:s}">{1:s}</a>'.format (
                    reichelt_id, reichelt.get ('article', ''))
                row['price']    = '{:.2f}'.format (price)
                row['subtotal'] = '{:.2f}'.format (subtotal)

        if octopart_id:
            row['octopart'] = '<a href="https://octopart.com/%s">link</a>' % octopart_id

        sheet_rows.append (td (row))
        sheet_total += subtotal
        total += subtotal

    rows.extend (sorted (sheet_rows, key = make_ref_sortable)) # this gets the 'refs' column sorted
    rows.append ('<td></td><td class="qty">{:d}</td><td class="sheet-total" colspan="10">{:.2f}</td>'.format (len (sheet_group), sheet_total))

rows.append ('<td></td><td class="qty">{:d}</td><td class="total" colspan="10">{:.2f}</td>'.format (len (components), total))

html = html.replace ('<!--TITLE-->', netlist.find ('.//title').text)
html = html.replace ('<!--ROWS-->',  tr (rows))

with codecs.open (fn_bom, 'w', encoding = 'utf-8') as fp:
    fp.write (html)

with open (fn_cache, 'w') as fp:
    json.dump (cache, fp)
