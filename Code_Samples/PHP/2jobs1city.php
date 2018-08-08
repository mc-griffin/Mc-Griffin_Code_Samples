<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>Graph the number of posting for a city</title>
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1" />
  
</head>
<body>

<h2>Compare 2 different jobs in one city</h2>
<br />

<SCRIPT LANGUAGE="JavaScript" SRC="CalendarPopup_Combined.js"></SCRIPT>
<SCRIPT LANGUAGE="JavaScript">
var cal = new CalendarPopup();
</SCRIPT>

<form name="search" action="2jobs1city_display.php" method="get" enctype="application/x-www-form-urlencoded">

<p>Choose job type 1 &nbsp;&nbsp;
<select name="job1">
<option value='acc'>accounting+finance</option>
<option value='ofc'>admin / office</option>
<option value='egr'>arch / engineering</option>
<option value='med'>art / media / design</option>
<option value='sci'>biotech / science</option>
<option value='bus'>business / mgmt</option>
<option value='csr'>customer service</option>
<option value='edu'>education</option>
<option value='etc'>et cetera jobs</option>
<option value='gov'>government</option>
<option value='hum'>human resources</option>
<option value='eng'>internet engineers</option>
<option value='lgl'>legal  /  paralegal</option>
<option value='mar'>marketing / pr / ad</option>
<option value='hea'>medical / health</option>
<option value='npo'>nonprofit sector</option>
<option value='ret'>retail / food / hosp</option>
<option value='sls'>sales / biz dev</option>
<option value='trd'>skilled trade / craft</option>
<option value='sof'>software / qa / dba</option>
<option value='sad'>systems / network</option>
<option value='tch'>technical support</option>
<option value='tfr'>tv / film / video</option>
<option value='art'>web / info design</option>
<option value='wri'>writing / editing</option>
</select></p>
<p>Search 1 &nbsp;&nbsp;
<INPUT TYPE="text" NAME="search1" VALUE="" SIZE="15"></p>

<p>Choose job type 2 &nbsp;&nbsp;
<select name="job2">
<option value='acc'>accounting+finance</option>
<option value='ofc'>admin / office</option>
<option value='egr'>arch / engineering</option>
<option value='med'>art / media / design</option>
<option value='sci'>biotech / science</option>
<option value='bus'>business / mgmt</option>
<option value='csr'>customer service</option>
<option value='edu'>education</option>
<option value='etc'>et cetera jobs</option>
<option value='gov'>government</option>
<option value='hum'>human resources</option>
<option value='eng'>internet engineers</option>
<option value='lgl'>legal  /  paralegal</option>
<option value='mar'>marketing / pr / ad</option>
<option value='hea'>medical / health</option>
<option value='npo'>nonprofit sector</option>
<option value='ret'>retail / food / hosp</option>
<option value='sls'>sales / biz dev</option>
<option value='trd'>skilled trade / craft</option>
<option value='sof'>software / qa / dba</option>
<option value='sad'>systems / network</option>
<option value='tch'>technical support</option>
<option value='tfr'>tv / film / video</option>
<option value='art'>web / info design</option>
<option value='wri'>writing / editing</option>
</select></p>
<p>Search 2 &nbsp;&nbsp;
<INPUT TYPE="text" NAME="search2" VALUE="" SIZE="15"></p>

<p>Choose a city &nbsp;&nbsp;
<?php
  $cities = GetCities();
  # print_r($cities);

  print '<select name="city">';
  foreach ($cities as $city) {
    printf("<option value='%s'>%s</option>", $city, $city);
  }
  print '</select>';
  print '<br /><br />';
?></p>

<p>Choose a section of the city &nbsp;&nbsp;
<INPUT TYPE="text" NAME="hood" VALUE="" SIZE="15"></p>

<p>Choose a start date &nbsp;&nbsp;
<INPUT TYPE="text" NAME="date1" VALUE="" SIZE="15">
<A HREF="#"
onClick="cal.select(document.forms['search'].date1,'anchor1','d-M-y');return false;"
NAME="anchor1" ID="anchor1">select</A></p>
<p>Choose a stop date &nbsp;&nbsp;
<INPUT TYPE="text" NAME="date2" VALUE="" SIZE="15">
<A HREF="#"
onClick="cal.select(document.forms['search'].date2,'anchor2','d-M-y');return false;"
NAME="anchor2" ID="anchor2">select</A></p>
<p>Display the results</p>
<input type="radio" name="display" value="number" checked="on">
Number of matches
<br />
<input type="radio" name="display" value="graph">
Graph of type: (need javascript to enable)
<br />
<input type="checkbox" name="graph_bar" value="bar">
bar
<br />
<input type="checkbox" name="graph_line" value="line" checked="on">
line
<br />
<br />
<input type="submit" value="Display">
</form>
<br />

</body>
</html>

<?php

function GetCities()
{
  $cities = array();

  $conn = oci_connect('craigtest', 'password', 'username');
  if (!$conn) {
    $e = oci_error();
    print htmlentities($e['message']);
    exit;
  }

  $query = 'SELECT * FROM cities';

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
    $cities[] = $row[0];
  }
  # print_r($cities);

  oci_close($conn);

  return $cities;
}
?>
