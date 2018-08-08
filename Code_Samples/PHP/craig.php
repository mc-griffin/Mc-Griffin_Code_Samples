<?php

// To display print statements, set $gPrint to true and do not call this file from <img src=>
$gPrint = false;

$gCraigEpochUnix = mktime(0, 0, 0, 1, 1, 2007);
$gCraigEpochUnixStr ='Jan 1 2007 00:00:00';
$gCraigEpochOracleStr = '01-Jan-2007';


function InitJobs(&$city, &$hood, &$search, &$cat, &$oraDate1, &$oraDate2, &$tableName, $suffix="")
{
	assert(!empty($_GET['city'.$suffix]));

	$city = strtolower($_GET['city'.$suffix]);
	$hood = strtolower($_GET['hood'.$suffix]);
	$search = strtolower($_GET['search']);
	$cat = strtolower($_GET['cat']);
	$tableName = $city.'_jobs';

	if (empty($_GET['date1']))
	{
		global $gCraigEpochOracleStr;
		$oraDate1 = $gCraigEpochOracleStr;
	}
	else
		$oraDate1 = GetFormatedDate($_GET['date1']);

	if (empty($_GET['date2']))
		$oraDate2 = gmdate("d-M-y");
	else
		$oraDate2 = GetFormatedDate($_GET['date2']);

	assert(strtotime($oraDate1) <= strtotime($oraDate2));
/*
	print "city: $city<br />";
	print "hood: $hood<br />";
	print "search: $search<br />";
	print "cat: $cat<br />";
	print "oraDate1: $oraDate1<br />";
	print "oraDate2: $oraDate2<br />";
	print "tableName: $tableName<br />";
	print '<br />';
*/
}

// used by 2jobs1city
function InitJobs2(&$city, &$hood, &$search, &$cat, &$oraDate1, &$oraDate2, &$tableName, $suffix="")
{
	assert(!empty($_GET['city']));

	$city = strtolower($_GET['city']);
	$hood = strtolower($_GET['hood']);
	$search = strtolower($_GET['search'.$suffix]);
	$cat = strtolower($_GET['job'.$suffix]);
	$tableName = $city.'_jobs';

	if (empty($_GET['date1']))
	{
		global $gCraigEpochOracleStr;
		$oraDate1 = $gCraigEpochOracleStr;
	}
	else
		$oraDate1 = GetFormatedDate($_GET['date1']);

	if (empty($_GET['date2']))
		$oraDate2 = gmdate("d-M-y");
	else
		$oraDate2 = GetFormatedDate($_GET['date2']);

	assert(strtotime($oraDate1) <= strtotime($oraDate2));
/*
	print "city: $city<br />";
	print "hood: $hood<br />";
	print "search: $search<br />";
	print "cat: $cat<br />";
	print "oraDate1: $oraDate1<br />";
	print "oraDate2: $oraDate2<br />";
	print "tableName: $tableName<br />";
	print '<br />';
*/
}

function InitRfs(&$city, &$hood, &$search, &$lower, &$upper, &$oraDate1, &$oraDate2, &$tableName, $suffix="")
{
	assert(!empty($_GET['city'.$suffix]));

	$city = strtolower($_GET['city'.$suffix]);
	$hood = strtolower($_GET['hood'.$suffix]);
	$search = strtolower($_GET['search']);
	$tableName = $city.'_rfs';

	$lower = strtolower($_GET['lower']);
	assert(empty($lower) || is_numeric($lower));
	$upper = strtolower($_GET['upper']);
	assert(empty($upper) || is_numeric($upper));

	if (empty($_GET['date1']))
	{
		global $gCraigEpochOracleStr;
		$oraDate1 = $gCraigEpochOracleStr;
	}
	else
		$oraDate1 = GetFormatedDate($_GET['date1']);

	if (empty($_GET['date2']))
		$oraDate2 = gmdate("d-M-y");
	else
		$oraDate2 = GetFormatedDate($_GET['date2']);
	
	assert(strtotime($oraDate1) <= strtotime($oraDate2));
/*
	print "city: $city<br />";
	print "hood: $hood<br />";
	print "search: $search<br />";
	print "lower: $lower<br />";
	print "upper: $upper<br />";
	print "oraDate1: $oraDate1<br />";
	print "oraDate2: $oraDate2<br />";
	print "tableName: $tableName<br />";
	print '<br />';
*/
}

function InitRfs2(&$city, &$hood, &$search, &$lower, &$upper, &$oraDate1, &$oraDate2, &$tableName, $suffix="")
{
	assert(!empty($_GET['city']));

	$city = strtolower($_GET['city']);
	$hood = strtolower($_GET['hood']);
	$search = strtolower($_GET['search'.$suffix]);
	$tableName = $city.'_rfs';

	$lower = strtolower($_GET['lower'.$suffix]);
	assert(empty($lower) || is_numeric($lower));
	$upper = strtolower($_GET['upper'.$suffix]);
	assert(empty($upper) || is_numeric($upper));

	if (empty($_GET['date1']))
	{
		global $gCraigEpochOracleStr;
		$oraDate1 = $gCraigEpochOracleStr;
	}
	else
		$oraDate1 = GetFormatedDate($_GET['date1']);

	if (empty($_GET['date2']))
		$oraDate2 = gmdate("d-M-y");
	else
		$oraDate2 = GetFormatedDate($_GET['date2']);
	
	assert(strtotime($oraDate1) <= strtotime($oraDate2));
/*
	print "city: $city<br />";
	print "hood: $hood<br />";
	print "search: $search<br />";
	print "lower: $lower<br />";
	print "upper: $upper<br />";
	print "oraDate1: $oraDate1<br />";
	print "oraDate2: $oraDate2<br />";
	print "tableName: $tableName<br />";
	print '<br />';
*/
}

function MonthIntToStr($monthInt)
{
	$month = "";
	if ($monthInt == 1) $month = "Jan";
	else if ($monthInt == 2) $month = "Feb";
	else if ($monthInt == 3) $month = "Mar";
	else if ($monthInt == 4) $month = "Apr";
	else if ($monthInt == 5) $month = "May";
	else if ($monthInt == 6) $month = "Jun";
	else if ($monthInt == 7) $month = "Jul";
	else if ($monthInt == 8) $month = "Aug";
	else if ($monthInt == 9) $month = "Sep";
	else if ($monthInt == 10) $month = "Oct";
	else if ($monthInt == 11) $month = "Nov";
	else if ($monthInt == 12) $month = "Dec";
	else assert(false);
	return $month;
}

function GetFormatedDate($dateIn)
{
	$dateParts = explode("-", $dateIn);
	$day = sprintf("%02d", $dateParts[0]);
	$monthInt = (int) $dateParts[1];
	$year = $dateParts[2];
	// monthInt -> month
	$month = MonthIntToStr($monthInt);
/*
	print $tableName;
	print '<br />';
	print $day.'***'.$month.'***'.$year.'<br />';
*/
	$date = $day."-".$month."-".$year;
	return $date;
}

function GetJobDescription($cat)
{
	if ($cat == 'acc') return 'accounting+finance';
	else if ($cat == 'ofc') return 'admin / office';
	else if ($cat == 'egr') return 'arch / engineering';
	else if ($cat == 'med') return 'art / media / design';
	else if ($cat == 'sci') return 'biotech / science';
	else if ($cat == 'bus') return 'business / mgmt';
	else if ($cat == 'csr') return 'customer service';
	else if ($cat == 'edu') return 'education';
	else if ($cat == 'etc') return 'et cetera jobs';
	else if ($cat == 'gov') return 'government';
	else if ($cat == 'hum') return 'human resources';
	else if ($cat == 'eng') return 'internet engineers';
	else if ($cat == 'lgl') return 'legal  /  paralegal';
	else if ($cat == 'mar') return 'marketing / pr / ad';
	else if ($cat == 'hea') return 'medical / health';
	else if ($cat == 'npo') return 'nonprofit sector';
	else if ($cat == 'ret') return 'retail / food / hosp';
	else if ($cat == 'sls') return 'sales / biz dev';
	else if ($cat == 'trd') return 'skilled trade / craft';
	else if ($cat == 'sof') return 'software / qa / dba';
	else if ($cat == 'sad') return 'systems / network';
	else if ($cat == 'tch') return 'technical support';
	else if ($cat == 'tfr') return 'tv / film / video';
	else if ($cat == 'art') return 'web / info design';
	else if ($cat == 'wri') return 'writing / editing';
	return 'Unknown';
}

function GetJobsQuery($tableName, $hood, $cat, $search, $date1, $date2)
{
	$query = "select posted, count(posted), (posted - to_date('01-jan-1970','dd-mon-yyyy'))*86400 from ".$tableName." ";

	if (empty($tableName) && empty($cat) && empty($search) && empty($date1) && empty($date)) {
	  $query .= "group by posted order by posted";
	  return $query;
	}

	$query .= "where ";
	$addAnd = false;
	if (!empty($hood)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "hood = '".$hood."' ";
	  $addAnd = true;
	}
	if (!empty($cat)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "lower(cat) = '".$cat."' ";
	  $addAnd = true;
	}
	if (!empty($date1) && !empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "(posted between '".$date1."' and '".$date2."') ";
	  $addAnd = true;
	} else if (empty($date1) && !empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "posted <= $date2 ";
	  $addAnd = true;
	} else if (!empty($date1) && empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "$date1 <= posted ";
	  $addAnd = true;
	}
	if (!empty($search)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "lower(html) like '%".$search."%' ";
	  $addAnd = true;
	}
	$query .= "group by posted order by posted";

	return $query;
}

function GetRfsQuery($tableName, $hood, $search, $lower, $upper, $date1, $date2)
{
	$query = "select posted, count(posted), (posted - to_date('01-jan-1970','dd-mon-yyyy'))*86400 from ".$tableName." ";

	if (empty($tableName) && empty($search) && empty($lower) && empty($upper) && empty($date1) && empty($date)) {
		$query .= "group by posted order by posted";
		return $query;
	}

	$query .= "where ";
	$addAnd = false;
	if (!empty($hood)) {
		if ($addAnd) $query .= "and ";
		$query .= "hood = '".$hood."' ";
		$addAnd = true;
	}
	if (!empty($lower) || !empty($upper)) {
		if ($addAnd) $query .= "and ";
		$addAnd = true;
		if (empty($lower)) {
			$query .= "price <= $upper ";
		}
		else if (empty($upper)) {
			$query .= "$lower <= price ";
		}
		else {
			$query .= "(price between $lower and $upper) ";
		}
	}
	if (!empty($date1) && !empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "(posted between '".$date1."' and '".$date2."') ";
	  $addAnd = true;
	} else if (empty($date1) && !empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "posted <= $date2 ";
	  $addAnd = true;
	} else if (!empty($date1) && empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "$date1 <= posted ";
	  $addAnd = true;
	}
	if (!empty($search)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "lower(html) like '%".$search."%' ";
	  $addAnd = true;
	}
	$query .= "group by posted order by posted";

	return $query;
}

function GetJobsCountQuery($tableName, $hood, $cat, $search, $date1, $date2)
{
	$query = "select count(*) from ".$tableName." ";

	if (empty($tableName) && empty($cat) && empty($search) && empty($date1) && empty($date)) {
	  $query .= "group by posted order by posted";
	  return $query;
	}

	$query .= "where ";
	$addAnd = false;
	if (!empty($hood)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "hood = '".$hood."' ";
	  $addAnd = true;
	}
	if (!empty($cat)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "lower(cat) = '".$cat."' ";
	  $addAnd = true;
	}
	if (!empty($date1) && !empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "(posted between '".$date1."' and '".$date2."') ";
	  $addAnd = true;
	} else if (empty($date1) && !empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "posted <= $date2 ";
	  $addAnd = true;
	} else if (!empty($date1) && empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "$date1 <= posted ";
	  $addAnd = true;
	}
	if (!empty($search)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "lower(html) like '%".$search."%' ";
	  $addAnd = true;
	}

	return $query;
}

function GetRfsCountQuery($tableName, $hood, $search, $lower, $upper, $date1, $date2)
{
	$query = "select count(*) from ".$tableName." ";

	if (empty($tableName) && empty($cat) && empty($search) && empty($date1) && empty($date)) {
	  $query .= "group by posted order by posted";
	  return $query;
	}

	$query .= "where ";
	$addAnd = false;
	if (!empty($hood)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "hood = '".$hood."' ";
	  $addAnd = true;
	}
	if (!empty($search)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "lower(html) like '%".$search."%' ";
	  $addAnd = true;
	}
	if (!empty($lower) || !empty($upper)) {
		if ($addAnd) $query .= "and ";
		$addAnd = true;
		if (empty($lower)) {
			$query .= "price <= $upper ";
		}
		else if (empty($upper)) {
			$query .= "$lower <= price ";
		}
		else {
			$query .= "(price between $lower and $upper) ";
		}
	}
	if (!empty($date1) && !empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "(posted between '".$date1."' and '".$date2."') ";
	  $addAnd = true;
	} else if (empty($date1) && !empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "posted <= $date2 ";
	  $addAnd = true;
	} else if (!empty($date1) && empty($date2)) {
	  if ($addAnd) $query .= "and ";
	  $query .= "$date1 <= posted ";
	  $addAnd = true;
	}

	return $query;
}

function GetJobsData($tableName, $hood, $cat, $search, $oraDate1, $oraDate2, &$dates, &$postingCounts, &$unixDates)
{
	$conn = oci_connect('craigtest', 'password', 'username');
	if (!$conn) {
		$e = oci_error();
		print htmlentities($e['message']);
		exit;
	}

	$query = GetJobsQuery($tableName, $hood, $cat, $search, $oraDate1, $oraDate2);

	$stid = oci_parse($conn, $query);
	if (!$stid) {
		$e = oci_error($conn);
		print htmlentities($e['message']);
		exit;
	}

	$r = oci_execute($stid, OCI_DEFAULT);
	if (!$r) {
		$e = oci_error($stid);
		echo htmlentities($e['message']);
		exit;
	}

	while ($row = oci_fetch_array($stid, OCI_RETURN_NULLS)) {
		$dates[] = $row[0];
		$postingCounts[] = $row[1];
		$unixDates[] = $row[2];
	}
	assert((count($dates) == count($postingCounts)) && (count($postingCounts) == count($unixDates)));
	// assert(2 <= count($dates));

	oci_close($conn);

	global $gPrint;
	if ($gPrint)
	{
		$prevUnixDate = 0;
		print '<p>A. '.count($dates).' rows</p>';
		print '<table border="1">';
		for ($i = 0; $i < count($dates); ++$i) {
			print '<tr>';
			print '<td>'.$unixDates[$i].'</td>';
			print '<td>'.$dates[$i].'</td>';
			print '<td>'.date('M d Y H:i:s T', $unixDates[$i]).'</td>';
			print '<td>'.gmdate('M d Y H:i:s T', $unixDates[$i]).'</td>';
			print '<td>'.$postingCounts[$i].'</td>';
			if ($i == 0)
				print '<td>&nbsp;</td>';
			else {
				$diff = $unixDates[$i] - $prevUnixDate;
				print '<td>'.$diff.'</td>';
			}
			print '</tr>';
			$prevUnixDate = $unixDates[$i];
		}
		print '</table>';
	}
}

function GetRfsData($tableName, $hood, $search, $lower, $upper, $oraDate1, $oraDate2, &$dates, &$postingCounts, &$unixDates)
{
	$conn = oci_connect('craigtest', 'password', 'username');
	if (!$conn) {
		$e = oci_error();
		print htmlentities($e['message']);
		exit;
	}

	$query = GetRfsQuery($tableName, $hood, $search, $lower, $upper, $oraDate1, $oraDate2);

	$stid = oci_parse($conn, $query);
	if (!$stid) {
		$e = oci_error($conn);
		print htmlentities($e['message']);
		exit;
	}

	$r = oci_execute($stid, OCI_DEFAULT);
	if (!$r) {
		$e = oci_error($stid);
		echo htmlentities($e['message']);
		exit;
	}

	while ($row = oci_fetch_array($stid, OCI_RETURN_NULLS)) {
		$dates[] = $row[0];
		$postingCounts[] = $row[1];
		$unixDates[] = $row[2];
	}
	assert((count($dates) == count($postingCounts)) && (count($postingCounts) == count($unixDates)));
	// assert(2 <= count($dates));

	oci_close($conn);

	global $gPrint;
	if ($gPrint)
	{
		$prevUnixDate = 0;
		print '<p>A. '.count($dates).' rows</p>';
		print '<table border="1">';
		for ($i = 0; $i < count($dates); ++$i) {
			print '<tr>';
			print '<td>'.$unixDates[$i].'</td>';
			print '<td>'.$dates[$i].'</td>';
			print '<td>'.date('M d Y H:i:s T', $unixDates[$i]).'</td>';
			print '<td>'.gmdate('M d Y H:i:s T', $unixDates[$i]).'</td>';
			print '<td>'.$postingCounts[$i].'</td>';
			if ($i == 0)
				print '<td>&nbsp;</td>';
			else {
				$diff = $unixDates[$i] - $prevUnixDate;
				print '<td>'.$diff.'</td>';
			}
			print '</tr>';
			$prevUnixDate = $unixDates[$i];
		}
		print '</table>';
	}
}

function GetJobsCountData($tableName, $hood, $cat, $search, $oraDate1, $oraDate2)
{
	$query = GetJobsCountQuery($tableName, $hood, $cat, $search, $oraDate1, $oraDate2);

	$conn = oci_connect('craigtest', 'password', 'username');
	if (!$conn) {
		$e = oci_error();
		print htmlentities($e['message']);
		exit;
	}

	$stid = oci_parse($conn, $query);
	if (!$stid) {
		$e = oci_error($conn);
		print htmlentities($e['message']);
		exit;
	}

	$r = oci_execute($stid, OCI_DEFAULT);
	if (!$r) {
		$e = oci_error($stid);
		echo htmlentities($e['message']);
		exit;
	}

	$row = oci_fetch_array($stid, OCI_RETURN_NULLS);
	$count = $row[0];

	oci_close($conn);
	
	return $count;
}

function GetRfsCountData($tableName, $hood, $search, $lower, $upper, $oraDate1, $oraDate2)
{
	$query = GetRfsCountQuery($tableName, $hood, $search, $lower, $upper, $oraDate1, $oraDate2);

	$conn = oci_connect('craigtest', 'password', 'username');
	if (!$conn) {
		$e = oci_error();
		print htmlentities($e['message']);
		exit;
	}

	$stid = oci_parse($conn, $query);
	if (!$stid) {
		$e = oci_error($conn);
		print htmlentities($e['message']);
		exit;
	}

	$r = oci_execute($stid, OCI_DEFAULT);
	if (!$r) {
		$e = oci_error($stid);
		echo htmlentities($e['message']);
		exit;
	}

	$row = oci_fetch_array($stid, OCI_RETURN_NULLS);
	$count = $row[0];

	oci_close($conn);
	
	return $count;
}

function GetRandomColor()
{
	// unknown with either "light" or "dar" prefix: purple, yellow, orange, brown
	$colors = array("red", "blue", "green");
	return $colors[rand(0, count($colors)-1)];
}

/*
Notes on Daylights Savings Time

Daylight savings happened March 11 at 2 a.m.  (http://geography.about.com/cs/daylightsavings/a/dst.htm)

At 2 a.m. on the second Sunday in March, we set our clocks forward one hour ahead of standard time ("spring forward").

Pacific Standard Time (PST) becomes Pacific Daylight Time (PDT).

SQL> select to_char(posted, 'dd-mon-yyyy HH24:MI:SS'), count(posted), (posted - to_date('01-jan-1970
','dd-mon-yyyy'))*86400 from portland_jobs where lower(cat) = 'sof' and (posted between '11-Mar-2007
' and '19-Mar-2007') and lower(html) like '%python%' group by posted order by posted;

TO_CHAR(POSTED,'DD-M COUNT(POSTED)
-------------------- -------------
(POSTED-TO_DATE('01-JAN-1970','DD-MON-YYYY'))*86400
---------------------------------------------------
12-mar-2007 00:00:00             3
                                         1173657600

13-mar-2007 00:00:00             4
                                         1173744000

14-mar-2007 00:00:00             5
                                         1173830400
                                         
TO_CHAR(POSTED,'DD-M COUNT(POSTED)
-------------------- -------------
(POSTED-TO_DATE('01-JAN-1970','DD-MON-YYYY'))*86400
---------------------------------------------------
15-mar-2007 00:00:00             5
                                         1173916800

16-mar-2007 00:00:00             1
                                         1174003200

17-mar-2007 00:00:00             2
                                         1174089600

*/

function IsDst($unixTime)
{
	// doesn't work: localtime($unixTime, true)['tm_isdst']
	$ar = localtime($unixTime, true);
	return $ar['tm_isdst'] != 0;
}

function ToGmt($t)
{
	// date('Z') is timezone offset in seconds. The offset for timezones west of UTC
	// is always negative, and for those east of UTC is always positive.
	// date('Z') is -25200
	// return $t + date('Z', $t); // what I was working with
	// return $t - date('Z', $t); // seems like it should work but doesn't
	return $t + date('Z'); // why this seems to work I have no idea (HUGE AMOUNT OF TIME TO DISCOVER)
}

function GetUnixTime($dateStr)
{
	$dateParts = explode("-", $dateStr);
	$day = (int) $dateParts[0];
	$month = (int) $dateParts[1];
	$year = (int) $dateParts[2];
	$unixTime = mktime(0, 0, 0, $month, $day, $year);
	return $unixTime;
}

// $d1 and $d2 are either both empty or both of the form 'Jan 1 1970 hh:mm:ss'.
// This function uses LOCAL time.
function GetDaysRange($d1="", $d2="")
{
	assert(empty($d1) == empty($d2));
	if (empty($d1) && empty($d2))
	{
		if (empty($_GET['date1']))
		{
			global $gCraigEpochUnixStr;
			$d1 = $gCraigEpochUnixStr;
		}
		else
			$d1 = date('M d Y H:i:s', GetUnixTime($_GET['date1']));
		$d1 .= ' PDT';
			
		if (empty($_GET['date2']))
		{
			$tomorrow = strtotime(date('M d Y 00:00:00').' +1 day');
			$d2 = date('M d Y H:i:s', $tomorrow);
		}
		else
			$d2 = date('M d Y H:i:s', GetUnixTime($_GET['date2']));
		$d2 .= ' PDT';
	}

	global $gPrint;
	if ($gPrint)
	{
		print '$d1 is '.$d1.' ('.strtotime($d1).')<br />';
		print '$d2 is '.$d2.' ('.strtotime($d2).')<br />';
	}

	// set $days with unix times in local times
	$days = array();

	$d2_unix = ToGmt(strtotime($d2));
	
	// $d is the iterator and it contains a date string.  I use
	// strtotime($dateStr.' +1 day')) to increment the date.
	// This takes into account DST and leap years.
	$d = $d1;
	$d_unix = ToGmt(strtotime($d));
	while ($d_unix <= $d2_unix)
	{
		$days[] = $d_unix;

		// advance the day iterators
		$d = date('M d Y H:i:s T', strtotime($d.' +1 day'));
		$d_unix = ToGmt(strtotime($d));
	}

	// ceil() because
	// Mar 31 2007 (1175324400) - Mar 01 2007 (1172736000) = 2588400 and
	// 2588400/86400.0 returns 29.958333333333332
	// daylight savings can affect this calculation too
	assert(abs(ceil(($d2_unix - ToGmt(strtotime($d1)))/86400) - count($days)) <= 1);

	global $gPrint;
	if ($gPrint)
	{
		$prevUnixDate = 0;
		print '<p>D. '.count($days).' rows</p>';
		print '<table border="1">';
		for ($i = 0; $i < count($days); ++$i) {
			print '<tr>';
			print '<td>'.$days[$i].'</td>';
			print '<td>'.date('M d Y H:i:s T', $days[$i]).'</td>';
			print '<td>'.gmdate('M d Y H:i:s T', $days[$i]).'</td>';
			if ($i == 0)
				print '<td>&nbsp;</td>';
			else {
				$diff = $days[$i] - $prevUnixDate;
				print '<td>'.$diff.'</td>';
			}
			print '</tr>';
			$prevUnixDate = $days[$i];
		}
		print '</table>';
	}

	return $days;
}

function Normalize($unixDates, $postingCounts, &$x, &$y)
{
	assert(count($unixDates) == count($postingCounts) && count($unixDates) <= count($x));
	
	$unixDatesCount = count($unixDates);
	if ($unixDatesCount <= 0)
		return;
	
	$firstDate = $unixDates[0];
	$latestDate = $unixDates[$unixDatesCount-1];

	global $gPrint;
	if (false) // $gPrint)
	{
		print '$x[0] is '.$x[0].' '.date('M d Y H:i:s T', $x[0]).'<br />';
		print 'ToGmt($x[0]) is '.ToGmt($x[0]).' '.date('M d Y H:i:s T', ToGmt($x[0])).'<br /><br />';
		print '$unixDates[0] is '.$unixDates[0].' '.date('M d Y H:i:s T', $unixDates[0]).'<br />';
		print 'ToGmt($unixDates[0]) is '.ToGmt($unixDates[0]).' '.date('M d Y H:i:s T', ToGmt($unixDates[0])).'<br /><br />';
		print '$x[count($x)-1] is '.$x[count($x)-1].' '.date('M d Y H:i:s T', $x[count($x)-1]).'<br />';
		print 'ToGmt($x[count($x)-1]) is '.ToGmt($x[count($x)-1]).' '.date('M d Y H:i:s T', ToGmt($x[count($x)-1])).'<br /><br />';
		print '$unixDates[$unixDatesCount-1] is '.$unixDates[$unixDatesCount-1].' '.date('M d Y H:i:s T', $unixDates[$unixDatesCount-1]).'<br />';
		print 'ToGmt($unixDates[$unixDatesCount-1]) is '.ToGmt($unixDates[$unixDatesCount-1]).' '.date('M d Y H:i:s T', ToGmt($unixDates[$unixDatesCount-1])).'<br />';
	}

	assert($x[0] <= $firstDate && $latestDate <= $x[count($x)-1]);

	$i = 0;
	foreach ($x as $i_x)
	{
		if ($i_x < $firstDate || $latestDate < $i_x)
			$y[] = 0;
		else if ($i_x == $unixDates[$i])
		{
			$y[] = (int) $postingCounts[$i];
			++$i;
		}
		else
			$y[] = 0;
	}
	assert(count($x) == count($y));

	global $gPrint;
	if ($gPrint)
	{
		// print date_default_timezone_get().'<br />';
		$prevUnixDate = 0;
		print '<p>B. '.count($x).' rows</p>';
		print '<table border="1">';
		for ($i = 0; $i < count($x); ++$i) {
			print '<tr>';
			print '<td>'.date('M d Y H:i:s T', $x[$i]).'</td>';
			print '<td>'.gmdate('M d Y H:i:s T', $x[$i]).'</td>';
			print '<td>'.$x[$i].'</td>';
			print '<td>'.date('I', $x[$i]).'</td>';
			print '<td>'.$y[$i].'</td>';
			if ($i == 0)
				print '<td>&nbsp;</td>';
			else {
				$diff = $x[$i] - $prevUnixDate;
				print '<td>'.$diff.'</td>';
			}
			print '</tr>';
			$prevUnixDate = $x[$i];
		}
		print '</table>';
		print '<br />';	
		for ($i = 0; $i < count($x); ++$i)
			print $x[$i].', ';
		print '<hr />';
		for ($i = 0; $i < count($x); ++$i)
			print $y[$i].', ';
		print '<br /><br />';	
	}

	$isTrans = (IsDst($x[0]) != IsDst($x[count($x)-1]));
	for ($i = 0; $i < count($x); ++$i)
	{
		if ($isTrans)
		{
			// this works for PST->PDT.  Do I need something else for PDT->PST?
			$x[$i] += 28800;
			if (!IsDst($x[$i]))
				$x[$i] += 60;
		}
		else
			$x[$i] += 25200;
	}

	global $gPrint;
	if ($gPrint)
	{
		$prevUnixDate = 0;
		print '<p>C. '.count($x).' rows</p>';
		print '<table border="1">';
		for ($i = 0; $i < count($x); ++$i) {
			print '<tr>';
			print '<td>'.date('M d Y H:i:s T', $x[$i]).'</td>';
			print '<td>'.gmdate('M d Y H:i:s T', $x[$i]).'</td>';
			print '<td>'.$x[$i].'</td>';
			print '<td>'.$y[$i].'</td>';
			if ($i == 0)
				print '<td>&nbsp;</td>';
			else {
				$diff = $x[$i] - $prevUnixDate;
				print '<td>'.$diff.'</td>';
			}
			print '</tr>';
			$prevUnixDate = $x[$i];
			
			// if (($x[$i] % 86400) != 0) print ' '.gmdate('M d Y H:i:s', $x[$i]).' not a multiple of 86400<br />';
		}
		print '</table>';
		print '<br />';	
		for ($i = 0; $i < count($x); ++$i)
			print $x[$i].', ';
		print '<hr />';
		for ($i = 0; $i < count($x); ++$i)
			print $y[$i].', ';
	}
}

function GetLegend($city, $hood)
{
	$legend = $city;
	if (!empty($hood))
		$legend .= " ($hood)";
	return $legend;
}

function GetRfsLegend($search, $lower, $upper)
{
	if (empty($search) && empty($lower) && empty($upper))
		return ""; // ?
	
	$legend = "";
	if (!empty($search))
	{
		$legend .= $search;
		if (!empty($lower) || !empty($upper))
			$legend .= ' ';
	}
	if (!empty($lower) && !empty($upper))
		$legend .= "$$lower-$$upper";
	else if (!empty($lower))
		$legend .= ">=$$lower";
	else if (!empty($upper))
		$legend .= "<=$$upper";
	return $legend;
}

function Get1Job1CityTitle($city, $cat, $hood="", $search="", $oraDate1="", $oraDate2="")
{
	$desc = GetJobDescription($cat);
	$title = "'$desc' jobs in $city";
	if (!empty($hood))
		$title .= " ($hood)";
	if (!empty($oraDate1) && !empty($oraDate2))
		$title .= " from $oraDate1 to $oraDate2";
	else if (!empty($oraDate1))
		$title .= " from $oraDate1";
	else if (!empty($oraDate2))
		$title .= " up to $oraDate2";
	if (!empty($search))
		$title .= " matching '$search'";
	return $title;
}

function Get1Job2CitiesTitle($cat, $city1, $city2, $hood1="", $hood2="", $search="", $oraDate1="", $oraDate2="")
{
	return "1Job2Cities Title";
}

function Get2Jobs1CityTitle($cat1, $cat2, $city, $hood="", $search1="", $search2="", $oraDate1="", $oraDate2="")
{
	return "2Jobs1City Title";
}

function Get1Rfs1CityTitle($city, $hood="", $search="", $lower="", $upper="", $oraDate1="", $oraDate2="")
{
	$title = "Real estate in $city";
	if (!empty($hood))
		$title .= " ($hood)";
	if (!empty($lower) && !empty($upper))
		$title .= " between $$lower and $$upper";
	else if (!empty($lower))
		$title .= " $$lower and up";
	else if (!empty($upper))
		$title .= " $$upper or below";	
	if (!empty($oraDate1) && !empty($oraDate2))
		$title .= " from $oraDate1 to $oraDate2";
	else if (!empty($oraDate1))
		$title .= " from $oraDate1";
	else if (!empty($oraDate2))
		$title .= " up to $oraDate2";
	if (!empty($search))
		$title .= " matching '$search'";
	return $title;
}

function Get1Rfs2CitiesTitle($city1, $city2, $hood1="", $hood2="", $search="", $lower="", $upper="", $oraDate1="", $oraDate2="")
{
	return "1Rfs2Cities Title";
}

function Get2Rfs1CityTitle($city, $hood="", $search1="", $lower1="", $upper1="", $search2="", $lower2="", $upper2="", $oraDate1="", $oraDate2="")
{
	return "2Rfs1City Title";
}

?>
