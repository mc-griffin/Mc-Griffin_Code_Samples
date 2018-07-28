#!/usr/bin/python2.7
import sys
import os
import time
import mail # mine
import pymssql
import decimal
import shutil
import pysftp
from os import listdir
from os.path import isfile, join


# sftp
g_url = "sftp.rjobrien.com"
g_username = "company_SFTP"
g_keyPath = "/root/.ssh/id_rsa"

g_portfolioId = {"Synthym" : "3021", "Phoenix" : "3024" }

g_path = "/var/sambashares/RJO/From_Sftp_Server/"
g_DailyPdfPath = "/var/sambashares/Accounts/RJO/Daily Confirms/"
g_MonthlyPdfPath = "/var/sambashares/Accounts/RJO/Monthly Statements/"

g_dailyFileTemplate = "RJO_Daily_Confirmations"
g_monthlyFileTemplate = "RJO_Monthly_Confirmations"

g_subject = sys.argv[0] + ": fatal error"


def CheckInsertMarginsTiburon(dateMargins, portfolioName):
	
	portfolioId = g_portfolioId[portfolioName]
	
	returnMsg = "Actions taken for " + portfolioName + ":\r\n"
	
	conn = pymssql.connect(host='tiburon', database='Napa', as_dict=True)
	if conn is None:
		raise Exception("pymssql.connect() returned None!")

	cur = conn.cursor()
	if cur is None:
		raise Exception("conn.cursor() returned None!")
		
	for day in sorted(dateMargins.keys()):
		
		# 2013-12-05 was the day we started using RJO.  RJO files from before this date are irrelevant.
		if day < '2013-12-05':
			continue
		
		initialMarginInFile = dateMargins[day]['initialMargin']
		filePath = dateMargins[day]['filePath']
		
		if initialMarginInFile == '.00':
			initialMarginInFile = '0.00'

		assert(0 < len(day) and day.replace("-","").isdigit() and 0 < len(initialMarginInFile) and initialMarginInFile.replace(".","").replace(",","").isdigit() and 0 < len(filePath))
		
		select = "select * from MarginToEquity where Date='{0}' and PortfolioID={1}".format(day, portfolioId)
		
		cur.execute(select)
		
		row = cur.fetchone()
		if row is None:
			msg = "Not in Tiburon: Date: {0}, File: {1}, Margin: {2}".format(day, filePath, initialMarginInFile) 
			# print(msg)
			
			returnMsg += (msg + " \r\n")
			
			insert = "insert into MarginToEquity (PortfolioID,Date,Margin) values ({0},'{1}',{2})".format(portfolioId, day, initialMarginInFile)
			# print insert
			
			returnMsg += (insert + " \r\n")
			
			cur.execute(insert)
		else:
			originalInitialMarginInTiburon = row['Margin']
			# truncate to two decimals
			initialMarginInTiburon = str(decimal.Decimal(originalInitialMarginInTiburon).quantize(decimal.Decimal('0.01')))
			
			if initialMarginInTiburon != initialMarginInFile:
				msg = "DIFFER Date: {0}, File: {1}, InFile: {2}, Tiburon: {3}".format(day, filePath, initialMarginInFile, initialMarginInTiburon)
				# print(msg)
				
				update = "update MarginToEquity set Margin={0} where PortfolioID={1} and Date='{2}'".format(initialMarginInFile, g_portfolioId, day)
				# print update
				
				raise Exception(msg + "\r\n\r\n" + update)
				# cur.execute(update)

	conn.commit()
	conn.close()
	
	return returnMsg


def GetTradingDays(date1, date2):
	queryTemplate = \
"""
select substring(CONVERT(varchar,CalendarDate,111),1,12) as CalendarDate
from CalendarDays
where CalendarDate between '{0}' and '{1}'
order by CalendarDate
"""
	query = queryTemplate.format(date1, date2)

	conn = pymssql.connect(host='tiburon', user='don', password='mamm7oth', database='Napa', as_dict=True)
	if conn is None:
		raise Exception("pymssql.connect() returned None!")

	cur = conn.cursor()
	if cur is None:
		raise Exception("conn.cursor() returned None!")

	cur.execute(query)

	row = cur.fetchone()
	if row is None:
		raise Exception("query {} returned no rows: " + query)

	tradingDays = []

	while row:
		day = row['CalendarDate']
		day = day.replace("/", "-")
		tradingDays.append(day)
		row = cur.fetchone()
		
	conn.close()
	
	return tradingDays


# returns  {"Synthym", "Phoenix"} ->  {statementDate} -> {"filePath", "initialMargin" }
def GetDateMargins(path):

	dateSynthym = {}
	datePhoenix= {}
	
	files = sorted(os.listdir(path))
	for file in files:
		if not file.lower().endswith(".csv") or file.lower().find("_nmny_") < 0:
			continue
			
		filePath = path + file
		
		retTuple = GetFileInfo(filePath)
		statementDate = retTuple[0]
		retDict = retTuple[1]
		
		# Files before 2015-10-19 do not have a Systematic entry, so the number of keys should be 2 or 3
		assert(len(retDict.keys()) == 2)
		
		if not retDict.has_key("Synthym"):
			raise Exception("GetDateMargins(): file '{0}' has no entry for Synthym (46918)".format(filePath))
		if not retDict.has_key("Phoenix"):
			raise Exception("GetDateMargins(): file '{0}' has no entry for Phoenix (01642)".format(filePath))
		
		synthymDict = retDict["Synthym"]
		phoenixDict = retDict["Phoenix"]
		
		if not synthymDict.has_key("filePath") or not synthymDict.has_key("initialMargin"):
			raise Exception("GetDateMargins(): file '{0}' has missing information for Synthym (46918)".format(filePath))
		if not phoenixDict.has_key("filePath") or not phoenixDict.has_key("initialMargin"):
			raise Exception("GetDateMargins(): file '{0}' has missing information for Phoenix (01642)".format(filePath))		
		
		assert(filePath == synthymDict["filePath"])
		assert(filePath == phoenixDict["filePath"])
		
		# print(filePath + "\t" + statementDate + "\tSynthym:" + synthymDict["initialMargin"] + "\tPhoenix:" + phoenixDict["initialMargin"])
						
		dateSynthym[statementDate] = synthymDict
		datePhoenix[statementDate] = phoenixDict
		
	retDict = {}
	retDict["Synthym"] = dateSynthym
	retDict["Phoenix"] = datePhoenix
	
	return retDict


# returns (statementDate, {"Synthym", "Phoenix"} -> {"filePath", "initialMargin" })
def GetFileInfo(filePath):
	# filePath ends with YYYYMMDD.csv
	
	synthymInitialMargin = ""
	phoenixInitialMargin = ""

	year = filePath[-12:-8]
	month = filePath[-8:-6]
	day = filePath[-6:-4]
	if not year.isdigit() or not month.isdigit() or not day.isdigit():
		raise Exception("GetFileInfo(): unexcepted file name format: {0}".format(filePath))
	statementDate = year + "-" + month + "-" + day

	with open(filePath) as f:
		# I assume only one row has a "9Z" in field 7.
		# field 7 is "account_type_code" and "9Z" means "Converted to USD"
		count9Z = 0
		content = f.readlines()
		for row in content:
			row = row.replace("\n","")
			fields = row.split(",")
			
			# 46918 is Synthym and 01642 is Phoenix
			field4 = fields[4]
			if field4 != "46918" and field4 != "01642" and field4 != "01987":
				raise Exception("GetFileInfo(): field 4 is '{0}' and should be either 46918 (Synthym) or 01642 (Phoenix)".format(field4))
			
			# possible values are (at least): "F1", "F3", "91", "93", and, the one we want, ""9Z"
			field7 = fields[7]
			if field7 != "9Z":
				continue
			
			count9Z += 1
			
			field22 = fields[22]
			field34 = fields[34]
			
			if field22 != field34:
				# print("Field 22 {0}, field 34 {1}".format(field22, field34))
				
				raise Exception("GetFileInfo(): field 22 '{0}', field 34 '{1}'".format(field22, field34))
			else:
				# print(field22)
				pass

			if field4 == "46918":
				synthymInitialMargin = field22
				assert(0 < len(synthymInitialMargin))
			elif field4 == "01642":
				phoenixInitialMargin = field22
				assert(0 < len(phoenixInitialMargin))
			elif field4 != "01987":
				raise Exception("GetFileInfo(): unexpected field 4 '{0}' in {1}".format(count9Z, filePath))
		
	if count9Z != 2 and count9Z != 3:
		raise Exception("GetFileInfo(): has {0} rows that contain field 7 with '9Z' in file: {1} when it should have 2 or 3".format(count9Z, filePath))
	
	if len(synthymInitialMargin) == 0:
		raise Exception("GetFileInfo(): file: {0} has no initital margin for Synthym".format(filePath))
	if len(phoenixInitialMargin) == 0:
		raise Exception("GetFileInfo(): file: {0} has no initital margin for Phoenix".format(filePath))
	
	assert(0 < len(filePath) and 0 < len(statementDate))
	
	retDict = {}
	
	synthym = {}
	synthym["filePath"] = filePath
	synthym["initialMargin"] = synthymInitialMargin
	retDict["Synthym"] = synthym

	phoenix = {}
	phoenix["filePath"] = filePath
	phoenix["initialMargin"] = phoenixInitialMargin
	retDict["Phoenix"] = phoenix

	return (statementDate, retDict)


def Download(pathDownload):
	
	"""
	print "Download: path:'{0}', g_url:'{1}', g_username:'{2}', g_password:'{3}', g_keyPath:'{4}'"\
		.format(pathDownload, g_url, g_username, g_password, g_keyPath)
	"""

	try:
		# named parameters necessary
		with pysftp.Connection(g_url, username=g_username, private_key=g_keyPath) as sftp:
			# sftp.get_r('/', pathDownload) # Exception caught: OSError(17, 'File exists')
			sftp.get_r('.', pathDownload)

	except Exception as e:
		err = "Download(): Exception caught: " + repr(e)
		raise Exception(err)


def ChangePermissions(path):

	try:
		files = [ f for f in listdir(path) if isfile(join(path,f)) ]
			
		for file in files:
			# print file
			filePath = path + file
			os.chown(filePath, 1019, 501) # 1019 is jimmy's uid

	except Exception as e:
		err = "ChangePermissions(): Exception caught: " + repr(e)
		raise Exception(err)
		

def CopyToOtherDirectories(path, dailyPdfPath, monthlyPdfPath):
	
	try:
		files = [ f for f in listdir(path) if isfile(join(path,f)) ]
			
		for file in files:
			if not file.lower().endswith(".pdf"):
				continue

			# example source file name:  company_SFTP_statement_dstm_20150909.pdf
			day = file[-6:-4]
			month = file[-8:-6]
			year = file[-12:-8]
			
			if not day.isdigit() or not month.isdigit() or not year.isdigit():
				# there are no files that should trip this condition
				continue

			sourceFilePath = path + file
			
			# destination file postfix:  _MM_DD_YYYY.pdf
			if 0 < file.lower().find("_dstm_"):
				destFileName = "{0}_{1}_{2}_{3}.pdf".format(g_dailyFileTemplate, month, day, year)
				destFilePath = dailyPdfPath + destFileName
				shutil.copy2(sourceFilePath, destFilePath)
				os.chown(destFilePath, 1019, 501) # 1019 is jimmy's uid
			elif 0 < file.lower().find("_mstm_"):
				destFileName = "{0}_{1}_{2}_{3}.pdf".format(g_monthlyFileTemplate, month, day, year)
				destFilePath = monthlyPdfPath + destFileName
				shutil.copy2(sourceFilePath, destFilePath)
				os.chown(destFilePath, 1019, 501) # 1019 is jimmy's uid
			elif 0 < file.lower().find("_deqty_"):
				pass
			else:
				mail.Mail(sys.argv[0] + ": Notice", dateTime + ": Unexpected file: " + file, False)

	except Exception as e:
		err = "CopyToOtherDirectories('{0}','{1}'): Exception caught: ".format(sourceFilePath, destFilePath) + repr(e)
		raise Exception(err)


if __name__ == "__main__":

	try:
		Download(g_path)
		
		ChangePermissions(g_path)
		CopyToOtherDirectories(g_path, g_DailyPdfPath, g_MonthlyPdfPath)

		# dateMargins: (statementDateSynthym -> {"filePath", "initialMargin" }, statementDatePhoneix -> {"filePath", "initialMargin" })
		dateMargins = GetDateMargins(g_path)
		synthymDateMargins = dateMargins["Synthym"]
		phoenixDateMargins = dateMargins["Phoenix"]

		"""
		print("Synthym:")
		for statementDate in sorted(synthymDateMargins.keys()):
			print("{0} '{1}'".format(statementDate, synthymDateMargins[statementDate]["initialMargin"]))
		print("Phoenix:")
		for statementDate in sorted(phoenixDateMargins.keys()):
			print("{0} '{1}'".format(statementDate, phoenixDateMargins[statementDate]["initialMargin"]))
		if dateMargins.has_key("Systematic"):
			print("Systematic:")
			for statementDate in sorted(systematicDateMargins.keys()):
				print("{0} '{1}'".format(statementDate, systematicDateMargins[statementDate]["initialMargin"]))
		"""
		
		sortedSynthymDateMargins = sorted(synthymDateMargins.keys())
		synthymFirstDate = sortedSynthymDateMargins[0]
		synthymLastDate = sortedSynthymDateMargins[len(sortedSynthymDateMargins)-1]
		# print "Synthym FirstDate: '{0}', LastDate: '{1}'".format(synthymFirstDate, synthymLastDate)
		
		sortedPhoenixDateMargins = sorted(phoenixDateMargins.keys())
		phoenixFirstDate = sortedPhoenixDateMargins[0]
		phoenixLastDate = sortedPhoenixDateMargins[len(sortedPhoenixDateMargins)-1]
		# print "Phoenix FirstDate: '{0}', LastDate: '{1}'".format(phoenixFirstDate, phoenixLastDate)
				
		synthymTradingDays = GetTradingDays(synthymFirstDate, synthymLastDate)
		for tradingDay in synthymTradingDays:
			if not tradingDay in synthymDateMargins.keys():
				raise Exception("Trading day {0} has no Synthym file".format(tradingDay))
		
		phoenixTradingDays = GetTradingDays(phoenixFirstDate, phoenixLastDate)
		for tradingDay in phoenixTradingDays:
			if not tradingDay in phoenixDateMargins.keys():
				raise Exception("Trading day {0} has no Phoenix file".format(tradingDay))
		
		returnMsg = CheckInsertMarginsTiburon(synthymDateMargins, "Synthym")
		returnMsg += "\r\n\r\n"
		returnMsg += CheckInsertMarginsTiburon(phoenixDateMargins, "Phoenix")
		
		dateTime = time.strftime("%Y-%m-%d %H:%M:%S")
		mail.Mail(sys.argv[0] + ": Notice", dateTime + ":\r\n\r\n" + returnMsg, False)

	except Exception as e:
		dateTime = time.strftime("%Y-%m-%d %H:%M:%S")
		mail.Mail(g_subject, dateTime + ": " + repr(e))
		# print(g_subject + dateTime + ": " + repr(e))
		exit(0)

