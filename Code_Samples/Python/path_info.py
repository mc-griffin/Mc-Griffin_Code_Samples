import sys
import os
import shutil
from os import listdir
from os.path import isfile, join
from os import stat
from stat import *
import datetime
	
g_epoch = datetime.datetime(1970, 1, 1)


class PathInfo:
	
	def __init__(self, path):
		self.path = path
		self.lastModificationDateTime = GetLastModification(path)
		self.lastModificationTimeStamp = (self.lastModificationDateTime - g_epoch).total_seconds()
		self.lastModification = self.lastModificationDateTime.strftime('%Y-%m-%d %H:%M:%S')
		self.isReadable = GetIsReadable(path)
		self.isWritable = GetIsWritable(path)
		self.statInfo = os.stat(path)
		self.size = self.statInfo.st_size
		
	def Display(self):
		print("path: " + self.path)
		print("size: {}".format(self.size))
		print("lastModification: " + self.lastModification)
		print("lastModificationTimeStamp: {}".format(self.lastModificationTimeStamp))
		print("isReadable: " + self.isReadable)
		print("isWritable: " + self.isWritable)


def GetLastModification(path): # file or directory
	stamp = -1
	try:
		stamp = stat(path).st_mtime
	except Exception as e:
		print("Exception: " + repr(e))
		exit(1)
	return datetime.datetime.fromtimestamp(stamp)


def GetIsReadable(path):
	st = os.stat(path)
	return str(bool(st.st_mode & S_IRUSR))


def GetIsWritable(path):
	st = os.stat(path)
	return str(bool(st.st_mode & S_IWGRP)) # group


if __name__ == "__main__":
	
	filePath = "F:\\app\\AccountType.txt"
	print(filePath)
	
	pathInfo = PathInfo(filePath)
	pathInfo.Display()
	

