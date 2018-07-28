import sys
import os
import shutil
import datetime
from os import stat
from stat import *
import subprocess
import pathInfo


g_destPath = "C:\\File_Server"

g_sourcePaths = [("\\\\company.root\\applications\\", "applications"), ("\\\\company.root\\development\\", "development")]


def UpdateDestFilePaths(sourceFilePaths, tooLong=248):
	newSourceFilePaths = []
	updatedSourceFilePaths = []
	destTooLong = []

	for sourceFilePath in sourceFilePaths:
		destFilePath = GetDestPath(sourceFilePath.path)
		# Log("source: '{}'\r\n\tdest: '{}'".format(sourceFilePath.path, destFilePath))

		if tooLong < len(destFilePath):
			destTooLong.append(destFilePath)
			continue

		if os.path.exists(destFilePath):
			destTimeStamp = GetLastModificationTimeStamp(destFilePath)
			if destTimeStamp == sourceFilePath.lastModificationTimeStamp:
				continue
			updatedSourceFilePaths.append(destFilePath)
		else:
			newSourceFilePaths.append(destFilePath)

		try:
			shutil.copyfile(sourceFilePath.path, destFilePath)
			Log("copy {}".format(sourceFilePath.path))
		except Exception as e:
			Log("{} | {} | {} | {} | UpdateDestFilePaths Exception".format(len(sourceFilePath.path), sourceFilePath.path, len(destFilePath), destFilePath))
		
		SetLastModification(destFilePath, sourceFilePath.lastModificationTimeStamp)
		
		# check that modification time was set as expected
		checkTimeStamp = GetLastModificationTimeStamp(destFilePath)
		if checkTimeStamp != sourceFilePath.lastModificationTimeStamp:
			Log("{} | {} | {} | {} | UpdateDestFilePaths".format(destFilePath, sourceFilePath.lastModificationTimeStamp, checkTimeStamp, sourceFilePath.lastModificationTimeStamp - checkTimeStamp))

	return (newSourceFilePaths, updatedSourceFilePaths, destTooLong)


def SetLastModification(path, timeStamp):
	# print "SetLastModification('{}', {})".format(path, timeStamp)
	
	try:
		os.utime(path, (timeStamp, timeStamp))
	except Exception as e:
		Log("SetLastModification Exception: " + repr(e))
		Log("\tpath: '{}', timeStamp: '{}'".format(path, timeStamp))
		return -1

	# Check that timestamp was set as expected and if not then correct it.
	# I don't know why this is necessary.  See time_problem.txt
	getTimeStamp1 = GetLastModificationTimeStamp(path)
	timeStampDiff1 = timeStamp - getTimeStamp1
	if getTimeStamp1 != timeStamp:
		# print "SetLastModification({}, {}): timestamp is {} with a difference of {}".format(path, timeStamp, getTimeStamp1, timeStampDiff1)
		
		newTimeStamp = timeStamp + timeStampDiff1
		# Log("Set '{}' to {}".format(path, newTimeStamp))
		try:
			os.utime(path, (newTimeStamp, newTimeStamp))
		except Exception as e:
			print "SetLastModification({}, {}): quit".format(path, newTimeStamp, getTimeStamp1)
			return -1;

		# check that timestamp was set as expected
		getTimeStamp2 = GetLastModificationTimeStamp(path)
		timeStampDiff = timeStamp - getTimeStamp2
		if getTimeStamp2 == timeStamp:
			# Log("'{}' has expected timestamp {}".format(path, getTimeStamp2))
			pass
		else:
			print "BAD: '{}' timestamp is {}, quit".format(path, getTimeStamp2)
			return -1
	
	return timeStamp


def GetLastModificationTimeStamp(path): # file or directory
	stamp = -1
	try:
		stamp = stat(path).st_mtime
	except Exception as e:
		Log("GetLastModification Exception: " + repr(e))
		return 0
	return (datetime.datetime.fromtimestamp(stamp) - pathInfo.g_epoch).total_seconds()


def GetDestPath(sourcePath):
	destDir = ""
	for path in g_sourcePaths:	
		if sourcePath.startswith(path[0]):
			destRoot = os.path.join(g_destPath, path[1])
			if not destRoot.endswith("\\"):
				destRoot += "\\"
			destDir = sourcePath.replace(path[0], destRoot)
	if len(destDir) == 0:
		assert("GetDestPath: " + sourcePath)
	return destDir


def MakeNewDestDirPaths(sourceDirPaths, tooLong=248):
	if not os.path.exists(os.path.join(g_destPath, "applications")):
		os.mkdir(os.path.join(g_destPath, "applications"))
	if not os.path.exists(os.path.join(g_destPath, "development")):
		os.mkdir(os.path.join(g_destPath, "development"))
	
	newDestDirs = []
	destTooLong = []
	
	for sourceDir in sourceDirPaths:
		destDir = GetDestPath(sourceDir.path)
		if len(destDir) == 0:
			assert("MakeNewDestDirPaths 2: " + sourceDir.path)
		# Log("source: '{}'\n\tdest: '{}'".format(sourceDir.path, destDir))
		# Log("dest: '{}'".format(destDir))
		
		if tooLong < len(destDir):
			destTooLong.append(destDir)
			continue
		
		if not os.path.exists(destDir):
			try:
				if len(destDir) == 0:
					assert("MakeNewDestDirPaths 2: " + sourceDir.path)
				os.mkdir(destDir)
				newDestDirs.append(destDir)
			except Exception as e:
				Log("MakeNewDestDirPaths Exception: " + repr(e))
				Log("\tsource: '{}'".format(sourceDir.path))
				Log("\tdest: '{}'".format(destDir))

	return (newDestDirs, destTooLong)


def GetSourcePaths(sourcePath):
	filePaths = []
	dirPaths = []
	tooLong = []
	
	GetSourcePathsRecurse(sourcePath, filePaths, dirPaths, tooLong)
	
	return (filePaths, dirPaths, tooLong)


def GetSourcePathsRecurse(sourcePath, filePaths, dirPaths, tooLong, tooLongLen=248):
	
	if tooLongLen < len(sourcePath):
		tooLong.append(sourcePath)
		return

	dirPaths.append(pathInfo.PathInfo(sourcePath))
	
	for file in os.listdir(sourcePath):
	
		path = os.path.join(sourcePath, file)
		
		if tooLongLen < len(path):
			tooLong.append(path)
			continue
		
		if "Backup\\applications_" in path:
			continue
		if "Backup\\development_" in path:
			continue
		if "Shared\\Software\\" in path:
			continue
		if "Backups_PCs\\" in path:
			continue
		
		try:
			mode = os.stat(path).st_mode
			if S_ISDIR(mode):
				# It's a directory, recurse into it
				GetSourcePathsRecurse(path, filePaths, dirPaths, tooLong)
			elif S_ISREG(mode):
				filePaths.append(pathInfo.PathInfo(path))
			else:
				# Unknown file type, print a message
				Log("Unknown type: '{}'".format(path))
		except Exception as e:
			# shortcuts fail
			Log("Fail: '{}' {} '{}'".format(path, len(path), repr(e)))


def Log(s):
	proc = subprocess.Popen('whoami', stdout=subprocess.PIPE)
	whoami = proc.stdout.read().strip()
	now = str(datetime.datetime.now())
	msg = whoami + ", " + now + ": " + s + "\n"
	print(msg)
	with open("C:\\Users\\Administrator\\Desktop\\copy\\novato_to_greenbrae_log.txt", "a") as f:
		f.write(msg)


if __name__ == "__main__":
	
	startTime = datetime.datetime.now()
	
	sourceFilePaths = []
	sourceDirPaths = []
	sourceTooLong = []
	destTooLong = []
	
	for path in g_sourcePaths:
		filePaths, dirPaths, tooLong = GetSourcePaths(path[0])
		sourceFilePaths.extend(filePaths)
		sourceDirPaths.extend(dirPaths)
		sourceTooLong.extend(tooLong)
		
	Log("{} files".format(len(sourceFilePaths)))
	Log("{} directories".format(len(sourceDirPaths)))
	Log("{} too long".format(len(sourceTooLong)))
	Log("")
	
	newDestDirPaths, tooLong = MakeNewDestDirPaths(sourceDirPaths)
	destTooLong.extend(tooLong)
	Log("New directories: {}".format(len(newDestDirPaths)))
	Log("")
	
	newDestFilePaths, updatedDestFilePaths, tooLong = UpdateDestFilePaths(sourceFilePaths)
	destTooLong.extend(tooLong)
	Log("New files: {}".format(len(newDestFilePaths)))
	Log("Updated files: {}".format(len(updatedDestFilePaths)))
	Log("")

	endTime = datetime.datetime.now()
	elapsed = endTime - startTime
	hours, remainder = divmod(elapsed.total_seconds(), 3600)
	minutes, seconds = divmod(remainder, 60)
	Log("Time: %02d:%02d:%02d" % (int(hours), int(minutes), int(seconds)))
	Log("")

	Log("too long: {}".format(len(sourceTooLong) + len(destTooLong)))
	Log("sourceTooLong: {}".format(len(sourceTooLong)))
	for path in sourceTooLong:
		Log(path)
	Log("destTooLong: {}".format(len(destTooLong)))
	for path in destTooLong:
		Log(path)
