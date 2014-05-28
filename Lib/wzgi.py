"""
WSGI implementation for ZeroVM based web applications
"""
import sys
import os

__version__ = "1.0"


class WSGIServer(object):
    def __init__(self, application):
        self.application = application

    def run(self):

        environ = {'wsgi.input': sys.stdin,
                   'wsgi.errors': sys.stderr,
                   'wsgi.version': (1, 0),
                   'wsgi.multithread': False,
                   'wsgi.multiprocess': True,
                   'wsgi.run_once': True}

        if environ.get('HTTPS', 'off') in ('on', '1'):
            environ['wsgi.url_scheme'] = 'https'
        else:
            environ['wsgi.url_scheme'] = 'http'

        environ.update(os.environ)
        headers_set = []
        headers_sent = []

        def write(chunk):
            if not headers_set:
                raise AssertionError("write() before start_response()")

            elif not headers_sent:
                # Before the first output, send the stored headers
                status, response_headers = headers_sent[:] = headers_set
                sys.stdout.write('%s %s\r\n' % (environ['SERVER_PROTOCOL'], status))
                for header in response_headers:
                    sys.stdout.write('%s: %s\r\n' % header)
                sys.stdout.write('\r\n')

            sys.stdout.write(chunk)
            sys.stdout.flush()

        def start_response(status, response_headers, exc_info=None):
            if exc_info:
                try:
                    if headers_sent:
                        # Re-raise original exception if headers sent
                        raise exc_info[0], exc_info[1], exc_info[2]
                finally:
                    exc_info = None     # avoid dangling circular ref
            elif headers_set:
                raise AssertionError("Headers already set!")

            headers_set[:] = [status, response_headers]
            return write

        result = self.application(environ, start_response)
        try:
            for data in result:
                if data:    # don't send headers until body appears
                    write(data)
            if not headers_sent:
                write('')   # send headers now if body was empty
        finally:
            if hasattr(result, 'close'):
                result.close()
