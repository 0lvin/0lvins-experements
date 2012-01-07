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

from cvsExport import cvsExport
from topPage import topPage

class paymentCalc(webapp.RequestHandler):
	
	def get_content(self):
		
		template_values = {
		      'result': "",
		      'internalData': ""
		}

		path = os.path.join(os.path.dirname(__file__), 'templates/paymentCalc.html')
		return template.render(path, template_values)

	def gen_template(self, content):
		top = topPage()	
		
		template_values = {
		      'top' : top.get_content(self.request),
		      'left': "",
		      'content': content
		}
		
		path = os.path.join(os.path.dirname(__file__), 'templates/index.html')
		
		self.response.headers['Content-Type'] = 'text/html;charset=utf-8'
		self.response.out.write(template.render(path, template_values))
		
	def get(self):
		self.gen_template(self.get_content())
		
	def post(self):
		self.gen_template(self.post_content())	
		
	def post_content(self):
        
		if (self.request.params.get('cvs', None) is None or not self.request.params.get('cvs').file):
			self.error(400)
			self.response.out.write("cvs not specified!")
			return
			
		try:
			skip = int(self.request.get('skip'))
		except ValueError:
				self.error(400)
				self.response.out.write("skip not specified!")
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
			time = int(self.request.get('time'))
		except ValueError:
				self.error(400)
				self.response.out.write("time not specified!")
				return
				
		try:
			price = int(self.request.get('price'))
		except ValueError:
				self.error(400)
				self.response.out.write("price not specified!")
				return
			
		cvs = self.request.params.get('cvs').file.read()
		cvsparse = cvsExport()
		self.response.headers['Content-Type'] = "text/html"
		table = ""
		
		operatorsTime = {}
		checkError = ""
		
		table += "<tr>"
		
		for colindex in range(0, (max(price,number,time) + 1 )):
			if colindex == price:
				table += "<td>Цена</td>"
			elif colindex == time:
				table += "<td>Время</td>"
			elif colindex == number:
				table += "<td>Номер</td>"
			else:
				table += "<td>_____</td>"
		try:
			cleanCvs = cvsparse.recode(cvs, encodings)
		except ValueError:
			self.error(400)
			self.response.out.write("Broken encodings!")
			return
			
		resultedTable = cvsparse.parse(cleanCvs, split, skip)
		for row in resultedTable :
			errorInRow = False 
			table += "<tr>"
			
			try:
				if float(string.replace(row[price],',','.')) > 0 :
					currNum = row[number][:6]
					currTime = float(string.replace(row[time]," ",""))
					alreadyExist = False
					for k,v in operatorsTime.iteritems():
						if k == currNum :
							operatorsTime[currNum] += currTime
							alreadyExist = True
					if not alreadyExist:
						operatorsTime[currNum] = currTime
			except :
				errorInRow = True
				
			for col in row:
				if errorInRow:
					table += ("<td bgcolor='red'>" + col + "</td>")
				else:
					table += ("<td>" + col + "</td>")
					
			table += "</tr>"
			
		table += "</tr>"
		
		Restable = ""
		
		for k,v in operatorsTime.iteritems():
			Restable += ("<tr><td>" + k + "</td><td>" + str(v / 60) + "</td><td>Минут</td></tr>")
			
		template_values = {
		      'result': Restable,
		      'internalData': table
		}

		path = os.path.join(os.path.dirname(__file__), 'templates/paymentCalc.html')
		return template.render(path, template_values)
