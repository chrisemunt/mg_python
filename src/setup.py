from distutils.core import setup, Extension

module = Extension('mg_python',
                    include_dirs = ['./'],
                    extra_compile_args = ['-Wno-unused-variable', '-Wno-unused-but-set-variable'],
                    sources = ['mg_python.c', 'mg_dba.c'])

def main():
    setup(name="mg_python",
          version="2.2.45",
          description="Python interface to M/Cache",
          author="Chris Munt",
          author_email="cmunt@mgateway.com",
          ext_modules=[module])

if __name__ == "__main__":
    main()
