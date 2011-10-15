#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#  common.py
#  
#  Copyright 2011 Matthew Horsell <matthew.horsell@googlemail.com>
#  
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  
#  

def escapeSlash(name):
	name.replace('/\\g','\\\\');
	name.replace('/\//g','\\/');
	return (name)
	
def getPlaylistType(mimetype):
	if mimetype == 'audio/x-mpegurl':
		return ('m3u')
	if mimetype == 'audio/x-scpls':
		return ('pls')
	return ('')

def getYear(date):
	return (date)
def main():
	
	return 0

if __name__ == '__main__':
	main()

