# -*- coding: utf-8 -*-
#   Copyright (C) 2009  Denis Pauk
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Library General Public
#   License as published by the Free Software Foundation; either
#   version 2 of the License, or (at your option) any later version.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Library General Public License for more details.
#
#   You should have received a copy of the GNU Library General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

import cgi
import os
import string
import sys

from google.appengine.api import memcache
from google.appengine.ext.webapp import template
from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext import db
from google.appengine.ext.webapp.util import run_wsgi_app

from paymentCalc import paymentCalc
from forumPage import forumPage
from pricePage import pricePage

def dump(obj):
	for attr in dir(obj):
		print "obj.%s = %s" % (attr, getattr(obj, attr))

application = webapp.WSGIApplication(
                                     [
                                      ('/', pricePage),
                                      ('/forum', forumPage),
				      ('/paymentcalc', paymentCalc)],
                                     debug=True)

def real_main():
	run_wsgi_app(application)

def profile_main():
 	# This is the main function for profiling 
 	# We've renamed our original main() above to real_main()
 	import cProfile, pstats
 	prof = cProfile.Profile()
 	prof = prof.runctx("real_main()", globals(), locals())
 	print "<pre>"
 	stats_memcahed = memcache.get_stats()
        print "<b>Cache Hits:%s</b><br>" % stats_memcahed['hits']
    	print "<b>Cache Misses:%s</b><br><br>" % stats_memcahed['misses'] 	
 	stats = pstats.Stats(prof)
 	stats.sort_stats("cumulative")  # time Or cumulative
 	stats.print_stats(80)  # 80 = how many to print
 	# The rest is optional.
 	# stats.print_callees()
 	print "</pre>"
 
if __name__ == "__main__":
	profile_main()
