# Writes the program guide (resources/startupHints/locales/<lang>/<n>.html)
# from the texts of pages_es_en.py, pages_de_fr.py and pages_ja_zh.py.
# The pictures are in resources/startupHints/assets/guide.
# Run from anywhere: python3 tools/guide/gen.py
import os, html

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, '..', '..', 'resources', 'startupHints')
LANGS = ['es', 'en', 'de', 'fr', 'ja', 'zh']

def load(name):
    values = {}
    exec(open(os.path.join(HERE, name), encoding='utf8').read(), values)
    return values

pages = load('pages_es_en.py')['P']

for name, langs in (('pages_de_fr.py', ('DE', 'FR')), ('pages_ja_zh.py', ('JA', 'ZH'))):
    values = load(name)

    for lang in langs:
        assert len(values[lang]) == len(pages), (lang, len(values[lang]), len(pages))

        for page, (title, intro, items, tip) in zip(pages, values[lang]):
            page[lang.lower()] = {'title': title, 'intro': intro, 'items': items, 'tip': tip}

CSS = '''* { font-family: "Segoe UI", Arial, Helvetica, sans-serif; }
body { margin: 0 14px; color: #222; font-size: 15px; line-height: 1.45; }
.title { width: 100%; color: #3b5fa0; text-align: center; margin: 8px 0 6px 0; font-size: 26px; }
.intro { text-align: center; color: #444; margin: 0 0 10px 0; }
.image { text-align: center; margin: 8px 0; }
.image img { max-width: 100%; border-radius: 6px; box-shadow: 0 1px 4px rgba(0,0,0,.25); }
.image.narrow img { max-height: 380px; }
ul { margin: 6px 0 8px 0; padding-left: 22px; }
li { margin: 4px 0; }
b { color: #1f3d73; }
.tip { display: block; font-style: italic; font-size: 0.85rem; color: #555; margin-top: 8px;
       background: #f3f6fb; border-left: 4px solid #6682b5; padding: 6px 10px; }
'''

for lang in LANGS:
    folder = os.path.join(ROOT, 'locales', lang)
    if os.path.isdir(folder):
        for name in os.listdir(folder):
            if name.endswith('.html') and name[:-5].isdigit():
                os.remove(os.path.join(folder, name))

for lang in LANGS:
    folder = os.path.join(ROOT, 'locales', lang)
    os.makedirs(os.path.join(folder, 'css'), exist_ok=True)
    open(os.path.join(folder, 'css', 'style.css'), 'w', encoding='utf8').write(CSS)

    for number, page in enumerate(pages, 1):
        text = page[lang]
        images = ''.join('<div class="image%s"><img src="../../assets/guide/%s" alt=""></div>\n'
                         % (' narrow' if image in page.get('narrow', []) else '', image)
                         for image in page['images'])
        items = ''.join('<li>%s</li>\n' % item for item in text['items'])
        tip = '<span class="tip">%s</span>\n' % text['tip'] if text.get('tip') else ''
        intro = '<p class="intro">%s</p>\n' % text['intro'] if text.get('intro') else ''
        doc = '''<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>%s</title>
    <link rel="stylesheet" href="css/style.css" type="text/css">
    <meta name="viewport" content="width=device-width" />
</head>
<body>
<h1 class="title">%s</h1>
%s%s<ul>
%s</ul>
%s</body>
</html>
''' % (html.escape(text['title']), html.escape(text['title']), intro, images, items, tip)
        open(os.path.join(folder, '%d.html' % number), 'w', encoding='utf8').write(doc)

print('pages', len(pages))
