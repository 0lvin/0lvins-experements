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

from google.appengine.ext.webapp import template
from google.appengine.api import users
from google.appengine.ext import webapp

class topPage():
	
	def get_content(self, request):
		
		if users.get_current_user():
			url_login = users.create_logout_url(request.uri)
			url_logintext = 'Выйти'
    		else:
			url_login = users.create_login_url(request.uri)
			url_logintext = 'Вход'
   
		template_values = {
			'url_login': url_login,
			'url_logintext': url_logintext
		}

		path = os.path.join(os.path.dirname(__file__), 'templates/top.html')
		return template.render(path, template_values)
