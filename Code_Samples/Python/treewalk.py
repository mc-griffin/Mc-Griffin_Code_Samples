import os, sys
from stat import *

def walktree(top, callback):
	'''recursively descend the directory tree rooted at top,
	calling the callback function for each regular file'''

	for f in os.listdir(top):
		try:
			pathname = os.path.join(top, f)
			mode = os.stat(pathname).st_mode
			if S_ISDIR(mode):
				# It's a directory, recurse into it
				walktree(pathname, callback)
			elif S_ISREG(mode):
				# It's a file, call the callback function
				callback(pathname)
			else:
				# Unknown file type, print a message
				print("Unknown type: '{}'".format(pathname))
		except Exception as e:
			print("Fail: '{}' '{}'".format(f, repr(e)))

def visitfile(file):
	print(file)

if __name__ == '__main__':
	walktree("Z:\\", visitfile)