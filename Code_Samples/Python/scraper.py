import urllib
import os, sys
import time
import random

import categories
import cityToHoods

gSleepBeforeGet = 0.5


# To turn off gEndIfFound and gEndIfFound2 by setting them to empty strings
# stop searching index page for a city-category when this text is encountered
gEndIfFound = "<h4>Thu Aug 31</h4>"
# in case gEndIfFound is a date and no posting occurred on that date
gEndIfFound2 = "<h4>Wed Aug 30</h4>"


def HarvestUrls(city, html, urls):
    added = 0
    searchFor = r'<p><a href="http://%s.craigslist.org/' % city
    i = html.find(searchFor)
    while 0 < i:
        end = html.find(r'.html">', i)
        assert i < end
        url = html[i+12:end] + ".html"
        filePath = UrlToFilePath(url)
        if not os.path.exists(filePath):
            urls.append(url)
            added += 1
        i = html.find(searchFor, i + 1)
    return 0 < added

def UrlToFilePath(url):
    fileName = ""
    urlNoPro = url[7:len(url)-5]
    parts = urlNoPro.split("/")
    city = parts[0].split(".")[0]
    assert 0 < len(city)
    if len(parts) == 4:
        # sfbay_eby_sof_197521575.html
        # must handle craigslist errors like http://boston.craigslist.org//sof/197962051.html
        # that leads to a valid page http://boston.craigslist.org/sof/197962051.html
        hood = parts[1]
        if len(hood) == 0:
            hood = "-"
        assert 0 < len(parts[2]) and 0 < len(parts[3])
        fileName = "%s_%s_%s_%s.html" % (city, hood, parts[2], parts[3])
    elif len(parts) == 3:
        # atlanta_-_sof_198036563.html
        assert 0 < len(parts[1]) and 0 < len(parts[2])
        fileName = "%s_-_%s_%s.html" % (city, parts[1], parts[2])
    else:
        print "unexpected: %s" % urlNoPro
        assert False
    cityDir = city
    if city == "orangecounty":
        cityDir = "orangeco"
    elif city == "sfbay":
        cityDir = "sfbayarea"
    elif city == "washingtondc":
        cityDir = "washdc"
    filePath = r"%s\%s\%s" % (r"c:\project\pages", cityDir, fileName)
    return filePath

def UrlToFileName(url):
    fileName = ""
    urlNoPro = url[7:len(url)-5]
    parts = urlNoPro.split("/")
    city = parts[0].split(".")[0]
    assert 0 < len(city)
    if len(parts) == 4:
        # sfbay_eby_sof_197521575.html
        # must handle craigslist errors like http://boston.craigslist.org//sof/197962051.html
        # that leads to a valid page http://boston.craigslist.org/sof/197962051.html
        hood = parts[1]
        if len(hood) == 0:
            hood = "-"
        assert 0 < len(parts[2]) and 0 < len(parts[3])
        fileName = "%s_%s_%s_%s.html" % (city, hood, parts[2], parts[3])
    elif len(parts) == 3:
        # atlanta_-_sof_198036563.html
        assert 0 < len(parts[1]) and 0 < len(parts[2])
        fileName = "%s_-_%s_%s.html" % (city, parts[1], parts[2])
    else:
        print "unexpected: %s" % urlNoPro
        assert False
    return fileName
 
def GetSavePage(url):
    filePath = UrlToFilePath(url)

    if os.path.exists(filePath):
        fileName = UrlToFileName(url)
        print "%s (%s) already exists and this shouldn't happen" % (fileName, url)
        return 0

    html = GetPage(url)

    if 0 <= html.find("automatically blocked by craigslist due to abuse"):
        print url
        print html
        assert False
        return 0

    if IsPageUnusable(html):
        fileName = UrlToFileName(url)
        print "unusable: %s" % fileName
        return 0

    f = open(filePath, "w")
    f.write(html)
    f.close()

    # exception from chmod() for filePath like c:\project\pages\atlanta_-_rfs_199133173.html
    # SetCreationModificationTimes(filePath, html)

    return 1    

def GetPage(url):
    time.sleep(gSleepBeforeGet)
    html = ""
    more = True
    count = 0
    while more and count < 5:
        try:
            sock = urllib.urlopen(url)
            html = sock.read()
            sock.close()
            more = False
        except:
            print "error: %s" % url
            time.sleep(1)
            count += 1
            assert more
    if 0 < count and count < 5:
        print "got: %s" % url
    return html

def SetCreationModificationTimes(filePath, html):
    i = html.find("Date: ")
    assert 0 < i
    # assert html.find("Date: ", i + 1) < 0

    end = html.find("<br>", i)
    assert i < end

    dateString = html[i+6:end]
    # print "%d: %s %s" % (count, fileName, dateString)

    # dateString like: 2006-08-22,  9:17PM PDT
    dateParts = dateString.split()
    assert len(dateParts) == 3

    timeZone = dateParts[2]

    dateParts2 = dateParts[0].split("-")
    assert len(dateParts2) == 3
    year = dateParts2[0]
    month = dateParts2[1]
    day = dateParts2[2][:-1] # shave off trailing comma

    hourParts = dateParts[1].split(":")
    assert len(hourParts) == 2
    hour = hourParts[0]
    assert len(hourParts[1]) == 4
    minute = hourParts[1][:2]
    half = hourParts[1][2:]

    year = int(year)
    month = int(month)
    day = int(day)
    hour = int(hour)
    minute = int(minute)

    if half == "AM" and hour == 12:
        hour = 0
    
    if half == "PM" and hour != 12:
        hour += 12

    assert 2005 < year
    assert 1 <= month and month <= 12
    assert 1 <= day and day <= 31
    assert 0 <= hour and hour <= 23
    assert 0 <= minute and minute <= 59

    # set secs to the time at which the page was posted
    try:
        # the 7th and 8th entries are intentionally out of range
        timeTuple = (year, month, day, hour, minute, 0, -1, 0, -1)
        secs = time.mktime(timeTuple)
        # check that secs is a reasonable value
        t = time.localtime(secs)
        assert t[0] == timeTuple[0] and t[1] == timeTuple[1] and t[2] == timeTuple[2] and t[3] == timeTuple[3] and t[4] == timeTuple[4]
    except OverflowError:
        print "OverflowError from time.mktime()"
        sys.exit()
    except ValueError:
        print "ValueError from time.mktime()"
        sys.exit()
    except TypeError:
        print "TypeError from time.mktime()"
        sys.exit()
    except:
        print "exception from time.mktime()"
        sys.exit()

    # set the created and modified times of the page to when it was posted and make read-only
    count = 0
    more = True
    while more:
        try:
            os.chmod(filePath, stat.S_IWRITE)
            os.utime(filePath, (secs, secs))
            os.chmod(filePath, stat.S_IREAD)
            more = False
        except:
            print "%d exception setting creation/modification times for %s" % (count, filePath)
            if 5 < count:
                return
            time.sleep(1)
            count += 1

def IsPageUnusable(html):
    if 0 < html.find("<h2>This posting has expired.</h2>"):
        return True
    if 0 < html.find("<h2>This posting has been deleted by its author.</h2>"):
        return True
    if 0 < html.find("<h2>This posting has been removed by craigslist community.</h2>"):
        return True
    if 0 < html.find("<h2>This posting has been removed by craigslist staff.</h2>"):
        return True
    if 0 < html.find("<h2>This posting has been removed.</h2>"):
        return True
    if 0 < html.find("404 Error"):
        return True
    # some pages have only 6 bytes ("\r\n\r\n  ")
    if html.find("Date: ") < 0:
        return True
    return False


if __name__ == "__main__":

    print "\n"
    start = time.time()
    random.seed()

    # urls of pages that describe a job or a property
    urls = []

    # accessing the indexes in sequential order doesn't seem to cause a block
    for city in cityToHoods.cities:
        # print city
        for cat in categories.categories:
            count = 0
            urlBase = r"http://%s.craigslist.org/%s/" % (city, cat)
            url = urlBase + "index.html"
            more = True
            while more:
                print url
                
                html = GetPage(url)

                atTheEnd = 0 < len(gEndIfFound) and 0 <= html.find(gEndIfFound)
                if not atTheEnd and 0 < len(gEndIfFound2) and 0 <= html.find(gEndIfFound2):
                    atTheEnd = True

                # count must be bigger than zero in case the first page contains gEndIfFound
                if 0 < count and atTheEnd:
                    print "\tfound sentinel date"
                    break
    
                addedUrls = HarvestUrls(city, html, urls)
                # if no urls were added, then the page contains urls already on disk, so break
                if not addedUrls:
                    print "\tall urls on page are already on disk"
                    break

                count += 1
                url = urlBase + "index%d" % (count * 100) + ".html"

                iFind = html.find("<h4>")
                more = 0 < iFind and not atTheEnd
                if not more:
                    print "\tfound end of indexes or sentinel date"


    print "%d urls" % len(urls)

    urlsCount = len(urls)
    downloadsCount = 0

    while 0 < len(urls):
        index = random.randrange(len(urls))
        url = urls[index]
        print "%d. %s %s" % (len(urls), url, UrlToFileName(url))
        downloadsCount += GetSavePage(url)
        del  urls[index]

    print "%d pages scanned, %d pages downloaded" % (urlsCount, downloadsCount)
    stop = time.time()
    print "ran for %d minutes" % ((int(stop-start)+59)/60) # outer parens required
