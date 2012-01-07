# -*- coding: utf-8 -*-
#   Copyright (C) 2007  Denis Pauk
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
#CONFIG
##SQL COMMANDS
source_sql = "select * from CUSTOMER"
destin_sql = "INSERT INTO CUSTOMER (FIRST_NAME, LAST_NAME, ADDRESS, CITY, COUNTRY, BIRTH_DATE)  VALUES ("
##FireBird
fb_base_ip = "localhost"
fb_base_user = "SYSDBA"
fb_base_pass = "123"
fb_base_database_path = "~/test"
##MySQL
my_base_ip = "localhost"
my_base_user = "free"
my_base_pass = "some_pass"
my_base_database_path = "databasefortest"
#DOCUMENTATION
##Full documentaion on Russian for language you can on http://www.opennet.ru/docs/RUS/python/index.html
##Database administration tool for Firebird RDBMS - FlameRobin
##if found error 'not found *mx*' install all python-egenix-mx-base-dbg
##This script using  
##connect to firebird - python-kinterbasdb
##connect to mysql - python-mysqldb
##python --version = 2.5.1
##http://www.botik.ru/~rldp/mysql/mysqldev/glava05.htm 
##http://kinterbasdb.sourceforge.net/dist_docs/usage.html#tutorial_connect
#SCRIPT
import kinterbasdb
import _mysql

def conectFireBird(fhost,fpath,fuser,fpassword):
	'''connect to firebird '''
	# The server is named 'localhost'; the database file is at 'temp'.
	con = kinterbasdb.connect(host=fhost,database=fpath,user=fuser,password=fpassword)
	# Create a Cursor object that operates in the context of Connection con:
	cur = con.cursor()
	return cur

def conectMySQL(fhost,fuser,fpass,fdatabase):
	'''connect to mysql'''
	cursor = _mysql.connect(host=fhost,user=fuser,passwd=fpass,db=fdatabase)
	return cursor

def getFromOneToOther(inputcursor,outcursor):
	'''Copy data from one to other base'''
	# Execute the SELECT statement:
	inputcursor.execute(source_sql)
	# Retrieve all rows as a sequence and print that sequence:
	data=inputcursor.fetchone()
	while data:
		try:
			# generate comand for execute in other base
			result_Command = destin_sql
			non_first = 0
			for fild in data:
				if non_first==1:
					result_Command=result_Command+","
				else:
					non_first=1
				result_Command=result_Command+"'%s'"%fild
			result_Command=result_Command+");"
			#debug for command
			#print result_Command
			#start add row
			outcursor.query(result_Command)
			print outcursor.use_result()
		except:
			print 'Error with', data
		data=inputcursor.fetchone()

onecursor=conectFireBird(fb_base_ip,fb_base_database_path,fb_base_user,fb_base_pass)
othercursor=conectMySQL(my_base_ip,my_base_user,my_base_pass,my_base_database_path)
getFromOneToOther(onecursor,othercursor)
#othercursor.query("""SELECT * FROM CUSTOMER""")
#r=othercursor.use_result()
#print r.fetch_row()
