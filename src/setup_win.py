from distutils.core import setup, Extension

module = Extension('mg_python',
                    libraries = ['Ws2_32'],
                    sources = ['mg_python.c'])

def main():
    setup(name="mg_python",
          version="2.1.44",
          description="Python interface to M/Cache",
          author="Chris Munt",
          author_email="cmunt@mgateway.com",
          ext_modules=[module])

if __name__ == "__main__":
    main()
