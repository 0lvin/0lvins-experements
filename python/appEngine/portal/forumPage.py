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

from topPage import topPage
from google.appengine.ext.webapp import template
from google.appengine.api import users
from google.appengine.ext import webapp
from google.appengine.ext import db
from google.appengine.api import images
from time import gmtime, strftime

class Message(db.Model):
	head = db.StringProperty(multiline=False)
	path = db.StringProperty(multiline=False)
	subpath = db.StringProperty(multiline=False)
	author = db.UserProperty()
	content = db.StringProperty(multiline=True)
	date = db.DateTimeProperty(auto_now_add=True)
	
class messageDataBase():
	def search(self, path):
		messages_query = Message.all().order('date')
		messages_query.filter('path =', path)
		messages = messages_query.fetch(10)
		return messages
		
	def search_parent(self, path):
		messages_query = Message.all().order('date')
		messages_query.filter('subpath =', path)
		messages = messages_query.fetch(10)
		return messages
		
	def nav(self, path):
		messages = self.search(path)
		result = "";
		for message in messages:
			result += (
				"<li><a href='/forum?path=" +
				message.subpath +
				"' title='" +
				message.content +
				"'>" +
				message.head +
				"</a><ul>" +
				self.nav(message.subpath) +
				"</ul></li>"
				)
		return result

class NavPage():
	
	def get_content(self):
		db = messageDataBase()
		template_values = {
		      'nav': db.nav("")
		}

		path = os.path.join(os.path.dirname(__file__), 'templates/nav.html')
		return template.render(path, template_values)
	
class ContentPage():
	
	def get_content(self, path):
		db = messageDataBase()

		messages = db.search_parent(path)
		sub_messages = db.search(path)
  
		template_values = {
		      'messages': messages,
		      'curr_path': path,
		      'sub_messages': sub_messages
		}

		path = os.path.join(os.path.dirname(__file__), 'templates/content.html')
		return template.render(path, template_values)

class forumPage(webapp.RequestHandler):
  	def get_content(self):
		db = messageDataBase()
		
		path = self.request.get('path')
		
		top = topPage()	
		left = NavPage()
		content = ContentPage()
		
		template_values = {
		      'top' : top.get_content(self.request),
		      'left': left.get_content(),
		      'content': content.get_content(path)
		}
		path = os.path.join(os.path.dirname(__file__), 'templates/index.html')
		return template.render(path, template_values)
	
	def get(self):
		self.response.headers['Content-Type'] = 'text/html;charset=utf-8'
		self.response.out.write(self.get_content())
	   
	def post(self):
		message = Message()
	
		if users.get_current_user():
			message.author = users.get_current_user()
		message.head = self.request.get('head')
		message.content = self.request.get('content')
		message.path = self.request.get('path')
		new_head = string.replace(message.head, "_", "*")
		sub_path = self.request.get('path') + "_" + message.head
		message.subpath = sub_path
		message.put()
		self.redirect('/forum?path=' + cgi.escape(sub_path.encode('utf-8')))	
