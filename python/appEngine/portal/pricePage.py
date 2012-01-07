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
import csv

from google.appengine.api import memcache
from google.appengine.ext.webapp import template
from google.appengine.api import users
from google.appengine.ext import db
from google.appengine.ext import webapp

from cvsExport import cvsExport
from topPage import topPage

class Rubric(db.Model):
	title = db.StringProperty(multiline=False)

class SubRubric(db.Model):
	rubric = db.StringProperty(multiline=False)
	title = db.StringProperty(multiline=False)

class Tags(db.Model):
	title = db.StringProperty(multiline=False)

class Object(db.Model):
	rubric = db.StringProperty(multiline=False)
	title = db.StringProperty(multiline=False)
	object_id = db.StringProperty(multiline=False)
	price = db.StringProperty(multiline=False)
	tags = db.StringListProperty()

class tagsDB():
	
	max_count = 1000
	
	#rubric !
	rubric_key_function = '/hiden_function_result=rubric_get'
	rubric_keylist_function = '/hiden_function_result=rubric_get_list'
	rubric_key_namespace = 'rubric'
	
	def get_rubrics(self):
		"""
			get list rubrics as object list
		"""
		result = memcache.get(
			self.rubric_key_function,
			namespace=self.rubric_key_namespace)
		if result != None :
			return result
		else :
			rubric_query = Rubric.all()
			rubric_query.order('title')
			result = rubric_query.fetch(self.max_count)
			memcache.add(
				self.rubric_key_function,
				result,
				3600,
				namespace=self.rubric_key_namespace)	
			return result
	
	def get_rubrics_list(self):
		"""
			get list rubric
		"""
		result = memcache.get(
			self.rubric_keylist_function,
			namespace=self.rubric_key_namespace)
		if result != None :
			return result
		else :
			rubrics_list = self.get_rubrics()
		
			string_list = []
		
			if rubrics_list != None:
				for rubric in rubrics_list:
					string_list.append(rubric.title)
			memcache.add(
				self.rubric_keylist_function,
				string_list,
				3600,
				namespace=self.rubric_key_namespace)			
			return string_list
	
	def add_rubrics(self, rubric_name):
		"""
			add rubric to base
		"""
		if rubric_name == None:
			return False
		
		if rubric_name == "":
			return False
	
		rubrics_list = self.get_rubrics_list()
		
		#You sure need 99 rubrics?
		if len(rubrics_list) == self.max_count:
			return False
			
		if rubric_name in rubrics_list:
			return False
							
		rubric = Rubric()		
		rubric.title = rubric_name
		rubric.put()
		
		memcache.delete(
			self.rubric_key_function,
			namespace=self.rubric_key_namespace)
		#replace list value in memcahed		
		rubrics_list.append(rubric_name)
		memcache.set(
			self.rubric_keylist_function,
			rubrics_list,
			3600,
			namespace=self.rubric_key_namespace)
		return True
	
	def reset_rubrics(self):
		"""
			reset memcahed
		"""
		for rubric in self.get_rubrics_list():
			self.reset_subrubrics(rubric)
			
		memcache.delete(
			self.rubric_key_function,
			namespace=self.rubric_key_namespace)
		memcache.delete(
			self.rubric_key_function,
 			namespace=self.rubric_key_namespace)
	#subrubric !
	subrubric_key_function = '/hiden_function_result=subrubric_get/value='
	subrubric_keylist_function = '/hiden_function_result=subrubric_get_list/value='
	subrubric_key_namespace = 'subrubric'
	
	def get_subrubrics(self, rubric):
		"""
			return all subrubric by rubric as list object
		"""
		if rubric == None:
			return None
		if rubric == "":
			return None
		
		result = memcache.get(
				self.subrubric_key_function + rubric,
				namespace=self.subrubric_key_namespace)
		if result != None :
			return result
		else :
			subrubric_query = SubRubric.all()
			subrubric_query.order('title')
			subrubric_query.filter('rubric =', rubric)
			result = subrubric_query.fetch(self.max_count)
			memcache.add(
				self.subrubric_key_function + rubric,
				result,
				3600,
				namespace=self.subrubric_key_namespace)
			return result
		
	def get_subrubrics_list(self, rubric):
		"""
			return all subrubric as list
		"""
		if rubric == None:
			return None
		if rubric == "":
			return None
		
		result = memcache.get(
				self.subrubric_keylist_function + rubric,
				namespace=self.subrubric_key_namespace)
		if result != None :
			return result
		else :
			subrubrics_list = self.get_subrubrics(rubric)
		
			string_list = []
		
			if subrubrics_list != None:
				for subrubric in subrubrics_list:
					string_list.append(subrubric.title)

			memcache.add(
				self.subrubric_keylist_function + rubric,
				string_list,
				3600,
				namespace=self.subrubric_key_namespace)
			return string_list
	
	def add_subrubrics(self, rubric_name, subrubric_name):
		"""
			Add subrubric
		"""
		if rubric_name == None:
			return False
		
		if rubric_name == "":
			return False
	
		rubrics_list = self.get_rubrics_list()
	
		if not (rubric_name in rubrics_list):
			return False
			
		subrubrics_list = self.get_subrubrics_list(rubric_name)
		
		#You sure need 99 subrubrics?
		if len(subrubrics_list) == self.max_count:
			return False
			
		if subrubric_name in subrubrics_list:
			return False
			
		subrubric = SubRubric()
		subrubric.rubric = rubric_name
		subrubric.title = subrubric_name
		subrubric.put()
		
		memcache.delete(
			self.subrubric_key_function + rubric_name,
			namespace=self.subrubric_key_namespace)
		#replace list value in memcahed		
		subrubrics_list.append(subrubric_name)
		memcache.set(
				self.subrubric_keylist_function + rubric_name,
				subrubrics_list,
				3600,
				namespace=self.subrubric_key_namespace)
		return True
		
	def reset_subrubrics(self, rubric_name):
		"""
			reset subrubrin in rubric
		"""
		if rubric_name == None:
			return
		if rubric_name == '':
			return
		memcache.delete(
			self.subrubric_key_function + rubric_name,
			namespace=self.subrubric_key_namespace)
		memcache.delete(
			self.subrubric_keylist_function + rubric_name,
			namespace=self.subrubric_key_namespace)
						
	#objects
	def get_objects(self, fromKey = None, rubric = None, tags = [],  count = 10):		
		object_query = Object.all()
		object_query.order('__key__')
		
		if fromKey != None:
			key = db.Key(fromKey)
			object_query.filter('__key__ >', key)
			
		if rubric != None:
			object_query.filter('rubric =', rubric)
		
		if tags != []:
			object_query.filter('tags IN', tags)
			
		objects = object_query.fetch(count)
		return objects

	def add(self, title, object_id, price, rubric):
		"""
			Обновление тегов после этого получение тегов 100 записей ~ 98 секунд
			Использование сразу после получения тагов из функции 100 записей ~ 7532413 function calls (7477960 primitive calls) in 48.638 CPU seconds</h4>
			С использованием Memcahed(tags) сохраняеться только непосредственно результат 100 ~ 8094488 function calls (7859477 primitive calls) in 46.902 CPU seconds
			С использованием Memcahed(tags) сохраняеться список тегов + дополняеться при add 100 ~  764657 function calls (754239 primitive calls) in 4.942 CPU seconds
			Add object to DataBase
			correct code but needed rules for price
			currPrice = price		
			currPrice = string.replace(currPrice,',','.')
			currPrice = string.replace(currPrice,' ','')
			if float(currPrice) > 0 :
				object = Object()
				object.title = title
				object.object_id = object_id
				object.price = currPrice
				object.put()
			not correct, but....
		"""
		if title == None or title == '':
			return False

		if rubric == None or rubric == '':
			return False

		if price == None or price == '':
			return False
		
		if object_id == None or object_id == '':
			return False
			
		object = Object()
		object.rubric = rubric
		object.title = title
		object.object_id = object_id
		object.price = price
		
		object.tags =  self.compile_tags(object.title)
		
		object.put()
	
	
	#check text contein only chars?
	def is_real_text(self, text):
		"""
			check text for only chars
		"""
		for char in text:
			if char in string.punctuation:
				return False
			if char in string.digits:
				return False
			if char in string.whitespace:
				return False		
		return True
	
	def get_tags_from_text(self, text):
		"""
			gen tags from text
			
			Tags must consist only chars and first char must be upper
		"""
		real_tags = []
		
		if text == None:
			return real_tags
			
		maybe_tags_list = text.split(' ')
		for tag in maybe_tags_list:
			# not one char and first char must be Upper
			if len(tag) > 1 and tag[0:1] == tag[0:1].upper():
				if self.is_real_text(tag):
					real_tags.append(tag.lower())
		return real_tags
			
	#tags
	def compile_tags(self, title):
		"""
			This function must get new tags from full text
			
			return all tags exist in tittle
		"""
		if title == None:
			return
		
		real_tags = self.get_tags_from_text(title)
		
		full_tags_set = self.get_tags_set()
		
		real_tags_set = frozenset(real_tags)
		
		new_tags = real_tags_set - full_tags_set
		
		for tag in new_tags:
			if not self.add_tags(tag):
				real_tags.remove(tag)
		
		return real_tags
	
	tags_key_function = '/hiden_function_result=tags_get'
	tags_keylist_function = '/hiden_function_result=tags_get_list'
	tags_key_namespace = 'tags'
	
	def get_tags(self):
		"""
			return all tags exist now
		"""
		result = memcache.get(
				self.tags_key_function,
				namespace=self.tags_key_namespace)
		if result != None:
			return result
		else:
			tags_query = Tags.all()
			tags_query.order('title')
			result = tags_query.fetch(self.max_count)
			memcache.add(
				self.tags_key_function,
				result,
				3600,
				namespace=self.tags_key_namespace)
			return result
		
	def get_tags_set(self):
		"""
			return full set of tags
		"""		
		result = memcache.get(
				self.tags_keylist_function,
				namespace=self.tags_key_namespace)
		if result != None:
			return result
		else:
			tags_list = self.get_tags()
			string_list = []
			if tags_list != None:
				for tag in tags_list:
					string_list.append(tag.title)
			result = set(tags_list)
			memcache.add(
				self.tags_keylist_function,
				result,
				3600,
				namespace=self.tags_key_namespace)
			return result
	
	def add_tags(self, tag_name):
		
		if tag_name == None:
			return False
		
		if tag_name == "":
			return False
	
		tags_list = self.get_tags_set()
		
		#You sure need 99 tags?
		if len(tags_list) == self.max_count:
			return False
			
		if tag_name in tags_list:
			#tag already exist - it's not error
			return True
							
		tag = Tags()		
		tag.title = tag_name
		tag.put()
		#delete raw value in memcached
		memcache.delete(
			self.tags_key_function,
			namespace=self.tags_key_namespace)
		#replace list value in memcahed		
		tags_list.add(tag_name)
		memcache.set(
				self.tags_keylist_function,
				tags_list,
				3600,
				namespace=self.tags_key_namespace)
		return True
		
	def reset_tags(self):
		memcache.delete(
			self.tags_key_function,
			namespace=self.tags_key_namespace)
		memcache.delete(
			self.tags_keylist_function,
			namespace=self.tags_key_namespace)
			
	def clean_objects(self, fromKey = None, rubric = None, tags = [], count = 10):
		
		objects = self.get_objects( fromKey, rubric, tags, count)
		
		return db.delete(objects)
		
class pricePage(webapp.RequestHandler):
			
	def get_content(self):
			
		db = tagsDB()
		
		if self.request.get('command', None) == 'reset':
			db.reset_rubrics()
			db.reset_tags
			
		rubric = self.request.get('rubric', None)
		
		if rubric != None :
			return self.get_content_with_rubric( db, rubric)
		else :
			template_values = {
				'addrubric': True,
				'rubrics': db.get_rubrics_list()
			}
		
			path = os.path.join(os.path.dirname(__file__), 'templates/price.html')
			return template.render(path, template_values)
		
	def get_content_with_rubric(self, db, rubric):
		
		subrubric = self.request.get('subrubric', None)
		
		if subrubric != None:
			return self.get_content_with_subrubric( db, rubric, subrubric)
		else:
			template_values = {
				'rubric_name' : rubric,
				'addsubrubric': True,
				'subrubrics': db.get_subrubrics_list(rubric)
			}
		
			path = os.path.join(os.path.dirname(__file__), 'templates/price.html')
			return template.render(path, template_values)
			
	def get_content_with_subrubric(self, db, rubric, subrubric):
		count = 10
		
		last_key_str = self.request.get('key', None)
		command = self.request.get('command', None)
		tags_string = self.request.get('tag', None)
		
		#prepere to run logic
		if last_key_str == "":
			last_key_str = None
		if rubric == "":
			rubric = None
			
		if tags_string == "":
			tags_string = None
		
		if subrubric == "":
			subrubric = None
		
		objects = None
		
		fullrubricname = None
		
		#real search
		if rubric != None and subrubric != None:
			if tags_string != None:
				tags = tags_string.split(',')
			else:			
				tags = []
			
			fullrubricname = rubric + "/" + subrubric
			
			if command == "clean":
				db.clean_objects(last_key_str, fullrubricname, tags, count)
				
			objects = db.get_objects(last_key_str, fullrubricname, tags, count)
			
			if len(objects) == count:
				#if recieved more result 20 -> start from 0
      				last_key_str = str(objects[count-2].key())
      			else :
				last_key_str = None			
	
		#prepere to render
		if tags_string == None:
			tags_string = ""
			
		if subrubric == None:
			subrubric = ""
			
		if rubric == None:
			rubric = ""
		
		if last_key_str != None:
			real_next = "key=" + last_key_str + "&tag=" + tags_string + "&rubric=" + rubric + "&subrubric=" + subrubric
		else:
			real_next = None
		
		template_values = {
			'rubric_name' : rubric,
			'subrubric_name' : subrubric,
			'addobject': True,
			'fullnamerubric': fullrubricname,
			'objects': objects,
			'lastKey': real_next,
		}
		
		path = os.path.join(os.path.dirname(__file__), 'templates/price.html')
		return template.render(path, template_values)
		
	def gen_template(self, content, left):
		top = topPage()	
		
		template_values = {
			'top' : top.get_content(self.request),
			'left': left,
			'content': content
		}
		
		path = os.path.join(os.path.dirname(__file__), 'templates/index.html')
		
		self.response.headers['Content-Type'] = 'text/html;charset=utf-8'
		self.response.out.write(template.render(path, template_values))
		
	def get(self):
		db = tagsDB()		
		left= "<ul>"
		tags_string = self.request.get('tag', None)		
		if tags_string != None:
			tags_string += ',' 
		else:
			tags_string = ""
		
		curr_tags_list = tags_string.split(',')
			
		tags = db.get_tags_set()		
		key = self.request.get('key', '')
		rubric = self.request.get('rubric', '')
		subrubric = self.request.get('subrubric', '')
		
		sub_key = "?key="
		sub_key += key
		sub_key += "&rubric="
		sub_key += rubric
		sub_key += "&subrubric="
		sub_key += subrubric
 		try:
			if tags:		
				for tagName in tags:
					if not (tagName in curr_tags_list):
						left += "<li><a href='/"
						left += sub_key
						left +=	"&tag="
						left +=	tags_string
						left +=	tagName
						left +=	"' title='"
						left +=	tagName
						left +=	"'>"
						left +=	tagName
						left +=	"</a></li>"
		except :
			pass
			
		#add clean tags
		left += "<li><a href='/"
		left += sub_key
		left += "'>Reset tags</a></li>"
		left += "</ul>"
		return self.gen_template(self.get_content(), left)
		
	def post(self):
		self.gen_template(self.post_content(),"")
	
	def parseAndAdd(self, cvsString, splitby, fromline, toline, price, title, number, rubric):
		db = tagsDB()
		broken = []
		firstSplit = cvsString.split("\n")
		skiped = 0
		for linesplit in firstSplit:
			if skiped < fromline:
				skiped += 1
			elif skiped > toline:
				break
			else:
				skiped += 1
				row = linesplit.split(splitby)
				try:
					db.add(
						row[title].decode('utf8'),
						row[number].decode('utf8'),
						row[price].decode('utf8'),
						# Fix this - unicode -> utf -> unicode it's not cool
						rubric) 						
				except :
					print sys.exc_info()                                                                                              
                                        print row
                                        print price
					broken.append(row)
		return broken
		
	def post_content(self):
                type = self.request.params.get('type', None)
                if type == 'price' :
			return self.post_content_price()
		elif type == 'subrubric' :
			return self.post_content_subrubric()
		elif type == 'rubric' :
			return self.post_content_rubric()
		else:
			self.error(400)
			self.response.out.write("type not specified!")
			return

	def post_content_subrubric(self):
		
		rubric = self.request.params.get('rubric', None)
		
		if rubric == None:
			self.error(400)
			self.response.out.write("rubric not specified!")
			return
		
		if rubric == "":
			self.error(400)
			self.response.out.write("rubric not specified!")
			return

		subrubric = self.request.params.get('subrubric', None)
		
		if subrubric == None:
			self.error(400)
			self.response.out.write("subrubric not specified!")
			return
		
		if subrubric == "":
			self.error(400)
			self.response.out.write("subrubric not specified!")
			return		
		
		
		db = tagsDB()
		
		if not db.add_subrubrics(rubric, subrubric):
			self.error(400)
			self.response.out.write("Already Exist sub rubric")
			return
			
		template_values = {
			'rubric_name' : rubric,
			'addsubrubric': True,
			'subrubrics': db.get_subrubrics_list(rubric)
		}

		path = os.path.join(os.path.dirname(__file__), 'templates/price.html')
		return template.render(path, template_values)
					
	def post_content_rubric(self):
		
		rubric = self.request.params.get('rubric', None)
		
		if rubric == None:
			self.error(400)
			self.response.out.write("rubric not specified!")
			return
		
		if rubric == "":
			self.error(400)
			self.response.out.write("rubric not specified!")
			return
		
		db = tagsDB()
		
		if not db.add_rubrics(rubric):
			self.error(400)
			self.response.out.write("Already Exist rubric")
			return
			
		template_values = {
			'addrubric': True,
			'rubrics': db.get_rubrics_list()
		}

		path = os.path.join(os.path.dirname(__file__), 'templates/price.html')
		return template.render(path, template_values)
		
        def post_content_price(self):
		if (self.request.params.get('cvs', None) == None):		
			self.error(400)
			self.response.out.write("cvs not specified!")
			return

		try:
			cvs = self.request.params.get('cvs').file.read()
		except :
			self.error(400)
			self.response.out.write("CSV not specified!")
			return
				
		try:
			skipfrom = int(self.request.get('skipfrom'))
		except ValueError:
				self.error(400)
				self.response.out.write("skipfrom not specified!")
				return
			
		try:
			skipto = int(self.request.get('skipto'))
		except ValueError:
				self.error(400)
				self.response.out.write("skipto not specified!")
				return
		
		
		subrubric = self.request.get('subrubric')
		rubric = self.request.get('rubric')
		
		db = tagsDB()
		
		if subrubric not in db.get_subrubrics_list(rubric):
			self.error(400)
			self.response.out.write("Rubric not correct!")
			return
		
		encodings = self.request.get('encodings').encode('utf-8')
		split = self.request.get('split').encode('utf-8')
		
		try:
			number = int(self.request.get('number'))
		except ValueError:
				self.error(400)
				self.response.out.write("number not specified!")
				return
				
		try:
			title = int(self.request.get('title'))
		except ValueError:
				self.error(400)
				self.response.out.write("title not specified!")
				return
				
		try:
			price = int(self.request.get('price'))
		except ValueError:
				self.error(400)
				self.response.out.write("price not specified!")
				return
							
		self.response.headers['Content-Type'] = "text/html"
		table = ""
		
		checkError = ""
		
		table += "<tr>"
		
		for colindex in range(0, (max(price,number,title) + 1 )):
			if colindex == price:
				table += "<td>Цена</td>"
			elif colindex == title:
				table += "<td>Описание</td>"
			elif colindex == number:
				table += "<td>Номер</td>"
			else:
				table += "<td>_____</td>"
		try:
			cvsparse = cvsExport()
			cleanCvs = cvsparse.recode(cvs, encodings)
		except ValueError:
			self.error(400)
			self.response.out.write("Broken encodings!")
			return
		
		resultedTable = self.parseAndAdd(cleanCvs, split, skipfrom, skipto, price, title, number, rubric + "/" + subrubric)
		for row in resultedTable :
			table += "<tr>"
			
			for col in row:
				table += ("<td bgcolor='red'>" + col + "</td>")
					
			table += "</tr>"
			
		table += "</tr>"		
				
		template_values = {
		      'objects': db.get_objects(),
		      'lastKey': None,
		      'internalData': table
		}

		path = os.path.join(os.path.dirname(__file__), 'templates/price.html')
		return template.render(path, template_values)
