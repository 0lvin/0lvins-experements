#! /usr/bin/python2.7
# -*- coding: utf8 -*-
# based on http://williams.best.vwh.net/sunrise_sunset_algorithm.htm
import time
import math

N = 100 # time.localtime().tm_yday

print "Day from new year %s" % N

sunrise=True

longitude = 13.408056
latitude = 52.518611
zenith = 90.83333333333333

#convert the longitude to hour value and calculate an approximate time
lngHour = longitude / 15

if sunrise:
	t = N + ((6 - lngHour) / 24)
else:
	t = N + ((18 - lngHour) / 24)
#calculate the Sun's mean anomaly
M = (0.9856 * t) - 3.289

#calculate the Sun's true longitude
L = M + (1.916 * math.sin(math.radians(M))) + (0.020 * math.sin(math.radians(2 * M))) + 282.634

if L > 360:
	L = L - 360
elif L < 0:
	L = L + 360

#calculate the Sun's right ascension
RA = math.degrees(math.atan(0.91764 * math.tan(math.radians(L))))

if RA > 360:
	RA = RA - 360
elif RA < 0:
	RA = RA + 360

#right ascension value needs to be in the same quadrant as L

Lquadrant  = (math.floor( L/90)) * 90
RAquadrant = (math.floor(RA/90)) * 90
RA = RA + (Lquadrant - RAquadrant)

#right ascension value needs to be converted into hours

RA = RA / 15

#calculate the Sun's declination

sinDec = 0.39782 * math.sin(math.radians(L))
cosDec = math.cos(math.asin(sinDec))

#calculate the Sun's local hour angle

cosH = (math.cos(math.radians(zenith)) - (sinDec * math.sin(math.radians(latitude)))) / (cosDec * math.cos(math.radians(latitude)))

if (cosH >  1):
	print "the sun never rises on this location (on the specified date)"
	exit

if (cosH < -1):
	print "the sun never sets on this location (on the specified date)"

#finish calculating H and convert into hours

if sunrise:
	H = 360 - math.degrees(math.acos(cosH))
else:
	H = math.degrees(math.acos(cosH))

H = H / 15

#calculate local mean time of rising/setting

T = H + RA - (0.06571 * t) - 6.622

#adjust back to UTC

UT = T - lngHour
if UT < 0:
	UT = UT + 24
elif UT > 24:
	UT = UT - 24

# convert UT value to local time zone of latitude/longitude
localT = UT + 1

print localT