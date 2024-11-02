import os
import sys
import shutil
import subprocess
import pathlib
from base64 import b64encode
import argparse
import tinkerforge_util as tfutil

tfutil.create_parent_module(__file__, 'software')

from software.tfpp import tfpp

JS_ANALYZE = False

BUILD_DIR = pathlib.Path('..', 'build')

HTML_MINIFIER_TERSER_OPTIONS = [
    '--collapse-boolean-attributes',
    '--collapse-inline-tag-whitespace',
    '--collapse-whitespace',
    '--conservative-collapse',
    '--decode-entities',
    '--no-include-auto-generated-tags',
    '--minify-urls',
    '--prevent-attributes-escaping',
    '--process-conditional-comments',
    '--remove-attribute-quotes',
    '--remove-comments',
    '--remove-empty-attributes',
    '--remove-optional-tags',
    '--remove-redundant-attributes',
    '--remove-script-type-attributes',
    '--remove-style-link-type-attributes',
    '--sort-attributes',
    '--sort-class-name',
    '--trim-custom-fragments',
    '--use-short-doctype'
]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--js-source-map', action='store_true')
    parser.add_argument('--css-source-map', action='store_true')
    parser.add_argument('--no-minify', action='store_true')
    parser.add_argument('modules', nargs='+')
    args = parser.parse_args()

    try:
        shutil.rmtree('src_tfpp')
    except FileNotFoundError:
        pass

    print('tfpp...')
    for root, dirs, files in os.walk('src', followlinks=True):
        for name in files:
            src_path = pathlib.Path(root) / name

            if src_path.parts[:2] == ('src', 'modules') and src_path.parts[2] not in args.modules:
                continue

            src_tfpp_path = pathlib.Path('src_tfpp', *src_path.parts[1:])

            if src_path.suffix in ['.ts', '.tsx']:
                try:
                    tfpp(src_path, src_tfpp_path)
                except Exception as e:
                    print(f'Error: {e}', file=sys.stderr)
                    exit(42)
            else:
                src_tfpp_path.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src_path, src_tfpp_path)

    os.chdir('src_tfpp')

    try:
        shutil.rmtree(str(BUILD_DIR))
    except FileNotFoundError:
        pass

    BUILD_DIR.mkdir(parents=True)

    print('tsc...')
    subprocess.check_call([
        'npx',
        'tsc',
        '--build',
        'tsconfig.json'
    ], shell=sys.platform == 'win32')

    print('esbuild...')
    esbuild_args = [
        'npx',
        'esbuild',
        'main.tsx',
        f'--metafile={BUILD_DIR / "meta.json"}',
        '--bundle',
        '--target=es6',
        '--alias:argon2-browser=../node_modules/argon2-browser/dist/argon2-bundled.min.js',
        '--alias:jquery=../node_modules/jquery/dist/jquery.slim.min',
        f'--outfile={BUILD_DIR / "bundle.min.js"}'
    ]

    if JS_ANALYZE:
        esbuild_args += ['--analyze']

    if args.js_source_map:
        esbuild_args += ['--sourcemap=inline']

    if not args.no_minify:
        esbuild_args += ['--minify']

    subprocess.check_call(esbuild_args, shell=sys.platform == 'win32')

    print('sass...')
    scss_args = [
        'npx',
        'sass',
    ]

    if not args.css_source_map:
        scss_args += ['--no-source-map']

    scss_args += [
        'main.scss',
        str(BUILD_DIR / 'main.css')
    ]

    subprocess.check_call(scss_args, shell=sys.platform == 'win32')

    print('postcss...')
    subprocess.check_call([
        'npx',
        'postcss',
        str(BUILD_DIR / 'main.css'),
        '-o',
        str(BUILD_DIR / 'main.min.css')
    ], shell=sys.platform == 'win32')

    if args.css_source_map:
        css_src = (BUILD_DIR / 'main.min.css').read_text(encoding='utf-8')
        css_map = b64encode((BUILD_DIR / 'main.min.css.map').read_bytes()).decode('ascii')

        (BUILD_DIR / 'main.min.css.map').unlink()

        css_src = css_src.replace('sourceMappingURL=main.min.css.map', f'sourceMappingURL=data:text/json;base64,{css_map}')

        (BUILD_DIR / 'main.min.css').write_text(css_src, encoding='utf-8')

    print('html-minifier-terser...')
    subprocess.check_call([
        'npx',
        'html-minifier-terser'] +
        HTML_MINIFIER_TERSER_OPTIONS + [
        '-o',
        str(BUILD_DIR / 'index.min.html'),
        'index.html'
    ], shell=sys.platform == 'win32')

if __name__ == '__main__':
    try:
        main()
    except subprocess.CalledProcessError as e:
        print(e, file=sys.stderr)
        exit(42)
